#include "vk_rt_bvh.hpp"
#include "vk_buffers.hpp"

static std::unique_ptr<RHI_BUFFER> create_bvh_buffer(RHI_DEVICE* device, buffer_type type,
	buffer_memory_type memory_type, VkDeviceSize size) {
	RHI_BUFFER_DESC desc;
	desc.device = device;
	desc.type = type;
	desc.memory_type = memory_type;
	desc.length = static_cast<size_t>(size);
	desc.mips = 1;
	desc.resource_flags |= resource_flags_rt_as;
	return std::unique_ptr<RHI_BUFFER>(vk_buffers_create_linear(&desc));
}

static VkDeviceAddress buffer_address(const VK_DEVICE* device, const RHI_BUFFER* buffer) {
	VkBufferDeviceAddressInfo info{};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	info.buffer = *static_cast<const VK_BUFFER*>(buffer);
	VkDeviceAddress address = vkGetBufferDeviceAddress(*device, &info);
	if (address == 0)
		throw std::runtime_error("Failed to get acceleration structure buffer address");
	return address;
}

static VkDeviceAddress acceleration_structure_address(const VK_DEVICE* device, VkAccelerationStructureKHR handle) {
	VkAccelerationStructureDeviceAddressInfoKHR info{};
	info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	info.accelerationStructure = handle;
	VkDeviceAddress address = vkGetAccelerationStructureDeviceAddressKHR(*device, &info);
	if (address == 0)
		throw std::runtime_error("Failed to get acceleration structure address");
	return address;
}

static VkDeviceSize scratch_alignment(const VK_DEVICE* device) {
	VkPhysicalDeviceAccelerationStructurePropertiesKHR properties{};
	properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
	VkPhysicalDeviceProperties2 device_properties{};
	device_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	device_properties.pNext = &properties;
	vkGetPhysicalDeviceProperties2(device->physical_device, &device_properties);
	if (properties.minAccelerationStructureScratchOffsetAlignment == 0)
		throw std::runtime_error("Invalid acceleration structure scratch alignment");
	return properties.minAccelerationStructureScratchOffsetAlignment;
}

static VkDeviceAddress scratch_address(const VK_DEVICE* device, const RHI_BUFFER* buffer, VkDeviceSize alignment) {
	VkDeviceAddress address = buffer_address(device, buffer);
	return (address + alignment - 1) & ~(alignment - 1);
}

static VkAccelerationStructureKHR create_acceleration_structure(const VK_DEVICE* device, RHI_BUFFER* storage,
	VkAccelerationStructureTypeKHR type, VkDeviceSize size) {
	VkAccelerationStructureCreateInfoKHR info{};
	info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	info.buffer = *static_cast<VK_BUFFER*>(storage);
	info.size = size;
	info.type = type;
	VkAccelerationStructureKHR handle = VK_NULL_HANDLE;
	if (vkCreateAccelerationStructureKHR(*device, &info, nullptr, &handle) != VK_SUCCESS)
		throw std::runtime_error("Failed to create acceleration structure");
	return handle;
}

static void build_barrier(VkCommandBuffer command_buffer, bool before_build) {
	VkMemoryBarrier2 barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
	barrier.srcStageMask = before_build ? VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT | VK_PIPELINE_STAGE_2_HOST_BIT
		: VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
	barrier.srcAccessMask = before_build ? VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_HOST_WRITE_BIT
		: VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	barrier.dstStageMask = before_build ? VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR
		: VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	barrier.dstAccessMask = before_build ? VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR
		: VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
	VkDependencyInfo dependency{};
	dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependency.memoryBarrierCount = 1;
	dependency.pMemoryBarriers = &barrier;
	vkCmdPipelineBarrier2(command_buffer, &dependency);
}

static void write_instances(const RHI_RT_BVH_GEOMETRY_DESC* desc, VK_BVH_BUFFER* tlas, bool update) {
	VK_DEVICE* device = static_cast<VK_DEVICE*>(desc->device);
	std::vector<VkAccelerationStructureInstanceKHR> instances(tlas->instance_count);
	std::vector<bool> changed(tlas->instance_count, false);
	size_t offset = 0;
	for (size_t i = 0; i < desc->instance_info_count; ++i) {
		const RHI_RT_BVH_GEOMETRY_INSTANCE_DESC& group = desc->instance_info[i];
		if (!group.parent_bvh || (group.transforms_count && !group.transforms))
			throw std::runtime_error("Invalid acceleration structure instance group");
		const size_t first = update ? group.update_index : offset;
		if (first > instances.size() || group.transforms_count > instances.size() - first ||
			group.transforms_count > (1u << 24))
			throw std::runtime_error("Acceleration structure instance range is out of bounds");
		const VK_RT_BVH* blas = static_cast<const VK_RT_BVH*>(group.parent_bvh);
		if (blas->parent_device != device)
			throw std::runtime_error("Acceleration structure belongs to a different device");
		VkDeviceAddress address = acceleration_structure_address(device, *blas);
		for (size_t j = 0; j < group.transforms_count; ++j) {
			if (!group.transforms[j])
				throw std::runtime_error("Invalid acceleration structure instance transform");
			VkAccelerationStructureInstanceKHR& instance = instances[first + j];
			for (size_t row = 0; row < 3; ++row)
				for (size_t column = 0; column < 4; ++column)
					instance.transform.matrix[row][column] = group.transforms[j][column * 4 + row];
			instance.instanceCustomIndex = static_cast<uint32_t>(j);
			instance.mask = 0xff;
			instance.accelerationStructureReference = address;
			changed[first + j] = true;
		}
		offset += group.transforms_count;
	}
	if (!update && offset != instances.size())
		throw std::runtime_error("Acceleration structure instance count does not match the groups");
	VK_BUFFER* input = static_cast<VK_BUFFER*>(tlas->inputs.get());
	void* mapped = nullptr;
	if (vkMapMemory(*device, *input->memory_pool, 0, VK_WHOLE_SIZE, 0, &mapped) != VK_SUCCESS)
		throw std::runtime_error("Failed to map acceleration structure instances");
	for (size_t i = 0; i < instances.size(); ++i)
		if (changed[i])
			static_cast<VkAccelerationStructureInstanceKHR*>(mapped)[i] = instances[i];
	vkUnmapMemory(*device, *input->memory_pool);
}

RHI_RT_BVH* vk_rt_bvh_create(const RHI_RT_BVH_DESC* const desc) {
	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);
	ASSERT_PTR(desc->command_buffer);
	ASSERT_PTR(desc->vertex_buffer);
	if (desc->count == 0 || desc->count > UINT32_MAX)
		throw std::runtime_error("Invalid BLAS geometry count");
	VK_DEVICE* device = static_cast<VK_DEVICE*>(desc->device);
	VkCommandBuffer command_buffer = *static_cast<VK_COMMAND_BUFFER*>(desc->command_buffer);
	std::vector<VkAccelerationStructureGeometryKHR> geometries(desc->count);
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> ranges(desc->count);
	std::vector<uint32_t> primitive_counts(desc->count);
	for (size_t i = 0; i < desc->count; ++i) {
		const RHI_BUFFER* vertices = desc->vertex_buffer[i];
		const RHI_BUFFER* indices = desc->index_buffer ? desc->index_buffer[i] : nullptr;
		if (!vertices || vertices->format <= resource_format_none || vertices->format >= resource_format_count ||
			vertices->stride == 0 || vertices->length / vertices->stride == 0 ||
			vertices->length / vertices->stride > UINT32_MAX)
			throw std::runtime_error("Invalid BLAS vertex buffer");
		VkAccelerationStructureGeometryKHR& geometry = geometries[i];
		geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		auto& triangles = geometry.geometry.triangles;
		triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		triangles.vertexFormat = vk_resource_format_type[vertices->format];
		triangles.vertexData.deviceAddress = buffer_address(device, vertices);
		triangles.vertexStride = vertices->stride;
		triangles.maxVertex = static_cast<uint32_t>(vertices->length / vertices->stride - 1);
		size_t element_count = vertices->length / vertices->stride;
		triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
		if (indices) {
			if ((indices->format != resource_format_uint16 && indices->format != resource_format_uint32) ||
				indices->stride != (indices->format == resource_format_uint16 ? sizeof(uint16_t) : sizeof(uint32_t)))
				throw std::runtime_error("Invalid BLAS index buffer format");
			triangles.indexType = indices->format == resource_format_uint16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
			triangles.indexData.deviceAddress = buffer_address(device, indices);
			element_count = indices->length / indices->stride;
		}
		if (element_count / 3 > UINT32_MAX)
			throw std::runtime_error("BLAS primitive count is too large");
		primitive_counts[i] = static_cast<uint32_t>(element_count / 3);
		ranges[i].primitiveCount = primitive_counts[i];
	}
	VkAccelerationStructureBuildGeometryInfoKHR build{};
	build.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	build.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	build.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	build.geometryCount = static_cast<uint32_t>(geometries.size());
	build.pGeometries = geometries.data();
	VkAccelerationStructureBuildSizesInfoKHR sizes{};
	sizes.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	vkGetAccelerationStructureBuildSizesKHR(*device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build, primitive_counts.data(), &sizes);
	auto result = std::make_unique<VK_RT_BVH>();
	result->parent_device = device;
	result->storage = create_bvh_buffer(desc->device, buffer_type_bvh, buffer_memory_type_gpu_read_only, sizes.accelerationStructureSize);
	const VkDeviceSize alignment = scratch_alignment(device);
	result->scratch = create_bvh_buffer(desc->device, buffer_type_raw, buffer_memory_type_gpu_read_only, sizes.buildScratchSize + alignment - 1);
	result->set_handle(create_acceleration_structure(device, result->storage.get(), build.type, sizes.accelerationStructureSize));
	build.dstAccelerationStructure = *result;
	build.scratchData.deviceAddress = scratch_address(device, result->scratch.get(), alignment);
	const VkAccelerationStructureBuildRangeInfoKHR* range = ranges.data();
	build_barrier(command_buffer, true);
	vkCmdBuildAccelerationStructuresKHR(command_buffer, 1, &build, &range);
	build_barrier(command_buffer, false);
	return result.release();
}

RHI_BUFFER* vk_rt_bvh_build_geometry_instances(const RHI_RT_BVH_GEOMETRY_DESC* const desc) {
	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);
	ASSERT_PTR(desc->command_buffer);
	if (desc->total_instances == 0 || desc->total_instances > UINT32_MAX || (desc->instance_info_count && !desc->instance_info))
		throw std::runtime_error("Invalid TLAS instance count");
	VK_DEVICE* device = static_cast<VK_DEVICE*>(desc->device);
	VkCommandBuffer command_buffer = *static_cast<VK_COMMAND_BUFFER*>(desc->command_buffer);
	auto result = std::make_unique<VK_BVH_BUFFER>();
	result->parent_device = device;
	result->type = buffer_type_bvh;
	result->instance_count = static_cast<uint32_t>(desc->total_instances);
	result->allow_update = !desc->read_only;
	result->length = desc->total_instances * sizeof(VkAccelerationStructureInstanceKHR);
	result->inputs = create_bvh_buffer(desc->device, buffer_type_raw, buffer_memory_type_shared_rw, result->length);
	write_instances(desc, result.get(), false);
	VkAccelerationStructureGeometryKHR geometry{};
	geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	geometry.geometry.instances.data.deviceAddress = buffer_address(device, result->inputs.get());
	VkAccelerationStructureBuildGeometryInfoKHR build{};
	build.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	build.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	build.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	if (result->allow_update)
		build.flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	build.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	build.geometryCount = 1;
	build.pGeometries = &geometry;
	VkAccelerationStructureBuildSizesInfoKHR sizes{};
	sizes.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	vkGetAccelerationStructureBuildSizesKHR(*device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build, &result->instance_count, &sizes);
	result->storage = create_bvh_buffer(desc->device, buffer_type_bvh, buffer_memory_type_gpu_read_only, sizes.accelerationStructureSize);
	const VkDeviceSize alignment = scratch_alignment(device);
	const VkDeviceSize scratch_size = sizes.buildScratchSize > sizes.updateScratchSize ? sizes.buildScratchSize : sizes.updateScratchSize;
	result->scratch = create_bvh_buffer(desc->device, buffer_type_raw, buffer_memory_type_gpu_read_only, scratch_size + alignment - 1);
	result->set_handle(create_acceleration_structure(device, result->storage.get(), build.type, sizes.accelerationStructureSize));
	build.dstAccelerationStructure = *result;
	build.scratchData.deviceAddress = scratch_address(device, result->scratch.get(), alignment);
	VkAccelerationStructureBuildRangeInfoKHR range{};
	range.primitiveCount = result->instance_count;
	const VkAccelerationStructureBuildRangeInfoKHR* range_ptr = &range;
	build_barrier(command_buffer, true);
	vkCmdBuildAccelerationStructuresKHR(command_buffer, 1, &build, &range_ptr);
	build_barrier(command_buffer, false);
	return result.release();
}

void vk_rt_bvh_update_geometry_instances(const RHI_RT_BVH_GEOMETRY_DESC* const desc, RHI_BUFFER* const buffer) {
	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);
	ASSERT_PTR(desc->command_buffer);
	ASSERT_PTR(buffer);
	VK_DEVICE* device = static_cast<VK_DEVICE*>(desc->device);
	VK_BVH_BUFFER* tlas = static_cast<VK_BVH_BUFFER*>(buffer);
	if (!tlas->allow_update || device != tlas->parent_device || (desc->instance_info_count && !desc->instance_info))
		throw std::runtime_error("Invalid TLAS update");
	write_instances(desc, tlas, true);
	VkAccelerationStructureGeometryKHR geometry{};
	geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	geometry.geometry.instances.data.deviceAddress = buffer_address(device, tlas->inputs.get());
	VkAccelerationStructureBuildGeometryInfoKHR build{};
	build.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	build.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	build.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	build.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
	build.srcAccelerationStructure = *tlas;
	build.dstAccelerationStructure = *tlas;
	build.geometryCount = 1;
	build.pGeometries = &geometry;
	build.scratchData.deviceAddress = scratch_address(device, tlas->scratch.get(), scratch_alignment(device));
	VkAccelerationStructureBuildRangeInfoKHR range{};
	range.primitiveCount = tlas->instance_count;
	const VkAccelerationStructureBuildRangeInfoKHR* range_ptr = &range;
	VkCommandBuffer command_buffer = *static_cast<VK_COMMAND_BUFFER*>(desc->command_buffer);
	//build_barrier(command_buffer, true);
	vkCmdBuildAccelerationStructuresKHR(command_buffer, 1, &build, &range_ptr);
	//build_barrier(command_buffer, false);
}
