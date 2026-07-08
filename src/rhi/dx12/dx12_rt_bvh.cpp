#include "dx12_rt_bvh.hpp"
#include "dx12_buffers.hpp"
#include "dx12_command_queue.hpp"
#include "dx12_command_buffer.hpp"

RHI_RT_BVH* dx12_rt_bvh_create(const RHI_RT_BVH_DESC* const desc)
{
	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);
	ASSERT_PTR(desc->command_buffer);
	ASSERT_PTR(desc->vertex_buffer);

	ID3D12Device* i_device_0 = *static_cast<DX_DEVICE*>(desc->device);
	ASSERT_PTR(i_device_0);
	ID3D12CommandList* i_command_buffer_0 = *static_cast<DX_COMMAND_BUFFER*>(desc->command_buffer);
	ASSERT_PTR(i_command_buffer_0);

	Microsoft::WRL::ComPtr<ID3D12Device5> i_device;
	ASSERT_SUCCESS(i_device_0->QueryInterface(IID_PPV_ARGS(&i_device)));
	ASSERT_PTR(i_device);

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> i_command_buffer;
	ASSERT_SUCCESS(i_command_buffer_0->QueryInterface(IID_PPV_ARGS(&i_command_buffer)));
	ASSERT_PTR(i_command_buffer);

	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geoemtries_desc(desc->count);
	std::vector<DX_RESOURCE*> resources;
	std::vector<D3D12_RESOURCE_STATES> resources_states;
	resources_states.reserve(desc->count * 2);
	resources.reserve(desc->count*2);
	for (size_t i = 0; i < geoemtries_desc.size(); i++) {

		D3D12_RAYTRACING_GEOMETRY_DESC& geom_desc = geoemtries_desc.at(i);

		RHI_BUFFER* v_buffer = desc->vertex_buffer[i];
		RHI_BUFFER* i_buffer = desc->index_buffer[i];
		DX_BUFFER* vb_impl = static_cast<DX_BUFFER*>(v_buffer);
		ASSERT_PTR(vb_impl);
		DX_BUFFER* ib_impl = static_cast<DX_BUFFER*>(i_buffer);

		ID3D12Resource* i_vb = *vb_impl;
		ASSERT_PTR(i_vb);

		geom_desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geom_desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
		geom_desc.Triangles.VertexBuffer.StartAddress =
			i_vb->GetGPUVirtualAddress();
		geom_desc.Triangles.VertexBuffer.StrideInBytes = static_cast<UINT64>(vb_impl->stride);
		geom_desc.Triangles.VertexCount = static_cast<UINT>(vb_impl->length / vb_impl->stride);
		geom_desc.Triangles.VertexFormat = dx12_resource_format_type[vb_impl->format];

		resources.push_back(vb_impl);
		resources_states.push_back(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		if (ib_impl)
		{
			ID3D12Resource* i_ib = *ib_impl;
			ASSERT_PTR(i_ib);
			geom_desc.Triangles.IndexBuffer =
				i_ib->GetGPUVirtualAddress();
			geom_desc.Triangles.IndexCount = static_cast<UINT>(ib_impl->length / ib_impl->stride);
			geom_desc.Triangles.IndexFormat = dx12_resource_format_type[ib_impl->format];
			resources.push_back(ib_impl);
			resources_states.push_back(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}
	}

	// create blas
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.NumDescs = static_cast<UINT>(geoemtries_desc.size());
	inputs.pGeometryDescs = geoemtries_desc.data();

	// pre build info
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO blasInfo = {};
	i_device->GetRaytracingAccelerationStructurePrebuildInfo(
		&inputs,
		&blasInfo);

	RHI_BUFFER_DESC buffer_desc;
	buffer_desc.device = desc->device;
	buffer_desc.type = buffer_type_rt_bvh;
	buffer_desc.format = resource_format_none;
	buffer_desc.memory_type = buffer_memory_type_gpu_only;
	buffer_desc.length = blasInfo.ResultDataMaxSizeInBytes;
	buffer_desc.mips = 1;
	std::unique_ptr<DX_BUFFER> blas_buffer_impl;
	blas_buffer_impl.reset(dx12_buffers_create<DX_BUFFER>(&buffer_desc));
	buffer_desc.type = buffer_type_raw;
	buffer_desc.length = blasInfo.ScratchDataSizeInBytes;
	std::unique_ptr<DX_BUFFER> scratch_buffer_impl;
	scratch_buffer_impl.reset(dx12_buffers_create<DX_BUFFER>(&buffer_desc));

	ID3D12Resource *i_blas_buffer = *blas_buffer_impl;
	ASSERT_PTR(i_blas_buffer);
	ID3D12Resource *i_scratch_buffer = *scratch_buffer_impl;
	ASSERT_PTR(i_blas_buffer);

	// build
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc = {};
	build_desc.Inputs = inputs;
	build_desc.DestAccelerationStructureData = i_blas_buffer->GetGPUVirtualAddress();
	build_desc.ScratchAccelerationStructureData = i_scratch_buffer->GetGPUVirtualAddress();
	
	dx12_command_buffer_resource_barrier_transition_and_restore(*static_cast<DX_COMMAND_BUFFER*>(desc->command_buffer),
		resources,
		resources_states,
		[&]() {
			i_command_buffer->BuildRaytracingAccelerationStructure(&build_desc, 0, nullptr);
	});

	// UAV barrier BLAS
	D3D12_RESOURCE_BARRIER blas_barrier = {};
	blas_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	blas_barrier.UAV.pResource = i_blas_buffer;

	i_command_buffer->ResourceBarrier(1, &blas_barrier);
	DX_RT_BVH* result = new DX_RT_BVH();
	ASSERT_PTR(result);
	
	i_blas_buffer->AddRef();
	i_scratch_buffer->AddRef();
	result->set_handle(i_blas_buffer);
	result->scratch_handle.set_handle(i_scratch_buffer);
	return result;
}

RHI_BUFFER* dx12_rt_bvh_build_geometry_instances(const RHI_RT_BVH_GEOMETRY_DESC* const desc)
{
	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);
	ASSERT_PTR(desc->command_buffer);

	DX_DEVICE* device_impl = static_cast<DX_DEVICE*>(desc->device);
	ID3D12Device* i_device_0 = *device_impl;
	ASSERT_PTR(i_device_0);
	
	ID3D12CommandList* i_command_buffer_0 = *static_cast<DX_COMMAND_BUFFER*>(desc->command_buffer);
	ASSERT_PTR(i_command_buffer_0);

	Microsoft::WRL::ComPtr<ID3D12Device5> i_device;
	ASSERT_SUCCESS(i_device_0->QueryInterface(IID_PPV_ARGS(&i_device)));
	ASSERT_PTR(i_device);	
	
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> i_command_buffer;
	ASSERT_SUCCESS(i_command_buffer_0->QueryInterface(IID_PPV_ARGS(&i_command_buffer)));
	ASSERT_PTR(i_command_buffer);

	// TLAS
	RHI_BUFFER_DESC inputs_buffer_desc;
	inputs_buffer_desc.device = desc->device;
	inputs_buffer_desc.type = buffer_type_raw;
	inputs_buffer_desc.length = desc->total_instances * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);
	inputs_buffer_desc.memory_type = buffer_memory_type_shared_rw;
	inputs_buffer_desc.mips = 1;
	std::unique_ptr<DX_BUFFER> tlas_inputs_buffer_impl;
	tlas_inputs_buffer_impl.reset(dx12_buffers_create<DX_BUFFER>(&inputs_buffer_desc));
	ID3D12Resource* i_tlas_inputs_buffer = *tlas_inputs_buffer_impl;
	ASSERT_PTR(i_tlas_inputs_buffer);

	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instances;
	instances.reserve(desc->total_instances);
	size_t offset = 0;
	for (uint32_t i = 0; i < desc->instance_info_count; ++i) {

		RHI_RT_BVH_GEOMETRY_INSTANCE_DESC* instance_desc = (desc->instance_info + i);
		ASSERT_PTR(instance_desc->parent_bvh);

		ID3D12Resource* i_blas_buffer = *static_cast<DX_RT_BVH*>(instance_desc->parent_bvh);
		ASSERT_PTR(i_blas_buffer);

		dx12_helpers_copy_4x4Matrix_to_rt_instance(instance_desc->transforms, instance_desc->transforms_count, i_blas_buffer->GetGPUVirtualAddress(), i_tlas_inputs_buffer, offset);
		
		offset += (instance_desc->transforms_count * sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
	}

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlas_inputs = {};
	tlas_inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	tlas_inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	tlas_inputs.NumDescs = static_cast<UINT>(desc->total_instances);
	tlas_inputs.InstanceDescs = i_tlas_inputs_buffer->GetGPUVirtualAddress();
	tlas_inputs.Flags =
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	if (!desc->read_only) 
		tlas_inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO tlas_info = {};
	i_device->GetRaytracingAccelerationStructurePrebuildInfo(&tlas_inputs, &tlas_info);

	RHI_BUFFER_DESC buffer_desc;
	buffer_desc.device = desc->device;
	buffer_desc.type = buffer_type_rt_bvh;
	buffer_desc.memory_type = buffer_memory_type_gpu_only;
	buffer_desc.length = tlas_info.ResultDataMaxSizeInBytes;
	buffer_desc.mips = 1;
	std::unique_ptr<DX_BUFFER> tlas_buffer_impl;
	tlas_buffer_impl.reset(dx12_buffers_create<DX_BUFFER>(&buffer_desc));
	buffer_desc.type = buffer_type_raw;
	buffer_desc.length = tlas_info.ScratchDataSizeInBytes;
	std::unique_ptr<DX_BUFFER> scratch_buffer_impl;
	scratch_buffer_impl.reset(dx12_buffers_create<DX_BUFFER>(&buffer_desc));

	ID3D12Resource* i_tlas_buffer = *tlas_buffer_impl;
	ASSERT_PTR(i_tlas_buffer);
	ID3D12Resource* i_scratch_buffer = *scratch_buffer_impl;
	ASSERT_PTR(i_scratch_buffer);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasBuild = {};
	tlasBuild.Inputs = tlas_inputs;
	tlasBuild.DestAccelerationStructureData = i_tlas_buffer->GetGPUVirtualAddress();
	tlasBuild.ScratchAccelerationStructureData = i_scratch_buffer->GetGPUVirtualAddress();

	i_command_buffer->BuildRaytracingAccelerationStructure(&tlasBuild, 0, nullptr);

	// UAV barrier TLAS
	D3D12_RESOURCE_BARRIER tlas_barrier = {};
	tlas_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	tlas_barrier.UAV.pResource = i_tlas_buffer;
	i_command_buffer->ResourceBarrier(1, &tlas_barrier);

	DX_BVH_BUFFER* result = new DX_BVH_BUFFER();
	ASSERT_PTR(result);
	i_tlas_buffer->AddRef();
	i_scratch_buffer->AddRef();
	i_tlas_inputs_buffer->AddRef();
	result->set_handle(i_tlas_buffer);
	result->scratch_handle.set_handle(i_scratch_buffer);
	result->inputs_buffer_handle.set_handle(i_tlas_inputs_buffer);
	return result;
}

void dx12_rt_bvh_update_geometry_instances(const RHI_RT_BVH_GEOMETRY_DESC* const desc, RHI_BUFFER* const buffer) {

	ASSERT_PTR(desc);
	ASSERT_PTR(buffer);

	ID3D12CommandList* i_command_buffer_0 = *static_cast<DX_COMMAND_BUFFER*>(desc->command_buffer);
	ASSERT_PTR(i_command_buffer_0);

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> i_command_buffer;
	ASSERT_SUCCESS(i_command_buffer_0->QueryInterface(IID_PPV_ARGS(&i_command_buffer)));
	ASSERT_PTR(i_command_buffer);

	DX_BVH_BUFFER* buffer_impl = static_cast<DX_BVH_BUFFER*>(buffer);
	ID3D12Resource* i_tlas_buffer = *buffer_impl;
	ASSERT_PTR(i_tlas_buffer);

	ID3D12Resource* i_tlas_input_buffer = buffer_impl->inputs_buffer_handle.com_ptr.Get();
	ASSERT_PTR(i_tlas_input_buffer);

	ID3D12Resource* i_scratch_buffer = buffer_impl->scratch_handle.com_ptr.Get();
	ASSERT_PTR(i_scratch_buffer);

	for (uint32_t i = 0; i < desc->instance_info_count; ++i) {

		RHI_RT_BVH_GEOMETRY_INSTANCE_DESC* instance_desc = (desc->instance_info + i);
		ASSERT_PTR(instance_desc->parent_bvh);

		ID3D12Resource* i_blas_buffer = *static_cast<DX_RT_BVH*>(instance_desc->parent_bvh);
		ASSERT_PTR(i_blas_buffer);

		size_t offset = (instance_desc->update_index * sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
		dx12_helpers_copy_4x4Matrix_to_rt_instance(instance_desc->transforms, instance_desc->transforms_count, i_blas_buffer->GetGPUVirtualAddress(), i_tlas_input_buffer, offset);
	}

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC updateDesc = {};

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& tlas_inputs = updateDesc.Inputs;

	tlas_inputs.Type =
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	tlas_inputs.DescsLayout =
		D3D12_ELEMENTS_LAYOUT_ARRAY;

	tlas_inputs.InstanceDescs =
		i_tlas_input_buffer->GetGPUVirtualAddress();

	tlas_inputs.NumDescs = static_cast<UINT>(desc->instance_info_count);

	tlas_inputs.Flags =
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE |
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE |
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;

	updateDesc.ScratchAccelerationStructureData =
		i_scratch_buffer->GetGPUVirtualAddress();

	updateDesc.SourceAccelerationStructureData =
		i_tlas_buffer->GetGPUVirtualAddress();

	updateDesc.DestAccelerationStructureData =
		i_tlas_buffer->GetGPUVirtualAddress();

	updateDesc.Inputs.Flags =
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;

	i_command_buffer->BuildRaytracingAccelerationStructure(
		&updateDesc,
		0,
		nullptr);

	// UAV barrier TLAS
	D3D12_RESOURCE_BARRIER tlas_barrier = {};
	tlas_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	tlas_barrier.UAV.pResource = i_tlas_buffer;
	i_command_buffer->ResourceBarrier(1, &tlas_barrier);

}