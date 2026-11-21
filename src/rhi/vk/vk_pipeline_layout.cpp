#include "vk_pipeline_layout.hpp"


RHI_PIPELINE_LAYOUT* vk_pipeline_layout_create(const RHI_PIPELINE_LAYOUT_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	std::vector<VkDescriptorSetLayoutBinding> descriptor_ranges;
	descriptor_ranges.reserve(desc->descriptor_count);

	uint32_t bt = 0;
	size_t constant_shader_registers_count = 0;
	size_t sampler_shader_registers_count = 0;
	size_t read_only_shader_registers_count = 0;
	size_t rw_shader_registers_count = 0;
	size_t read_only_texture_shader_registers_count = 0;
	size_t rw_texture_shader_registers_count = 0;

	// first heap slot of each descriptor type. Resource types share one heap
	// (appended in declaration order), samplers live in their own heap.
	size_t resource_heap_slot = 0;
	size_t sampler_heap_slot = 0;
	size_t constant_heap_start = 0;
	size_t sampler_heap_start = 0;
	size_t read_only_heap_start = 0;
	size_t rw_heap_start = 0;
	size_t read_only_texture_heap_start = 0;
	size_t rw_texture_heap_start = 0;

	for (size_t i = 0; i < desc->descriptor_count; i++) {

		const RHI_SHADER_DESCRIPTOR_DESC& descriptor = desc->descriptors[i];
		VkDescriptorType range_type;
		switch (descriptor.shader_view_type) {
		case shader_view_type_read_only_texture_buffer:
			range_type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			if (read_only_texture_shader_registers_count == 0) read_only_texture_heap_start = resource_heap_slot;
			read_only_texture_shader_registers_count += descriptor.shader_register_max;
			resource_heap_slot += descriptor.shader_register_max;
			break;
		case shader_view_type_read_only_buffer:
			range_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			if (read_only_shader_registers_count == 0) read_only_heap_start = resource_heap_slot;
			read_only_shader_registers_count += descriptor.shader_register_max;
			resource_heap_slot += descriptor.shader_register_max;
			break;
		case shader_view_type_bvh_buffer:
			range_type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
			if (read_only_shader_registers_count == 0) read_only_heap_start = resource_heap_slot;
			read_only_shader_registers_count += descriptor.shader_register_max;
			resource_heap_slot += descriptor.shader_register_max;
			break;
		case shader_view_type_rw_texture_buffer:
			range_type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			if (rw_texture_shader_registers_count == 0) rw_texture_heap_start = resource_heap_slot;
			rw_texture_shader_registers_count += descriptor.shader_register_max;
			resource_heap_slot += descriptor.shader_register_max;
			break;
		case shader_view_type_rw_buffer:
			range_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			if (rw_shader_registers_count == 0) rw_heap_start = resource_heap_slot;
			rw_shader_registers_count += descriptor.shader_register_max;
			resource_heap_slot += descriptor.shader_register_max;
			break;
		case shader_view_type_constant_buffer:
			range_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			if (constant_shader_registers_count == 0) constant_heap_start = resource_heap_slot;
			constant_shader_registers_count += descriptor.shader_register_max;
			resource_heap_slot += descriptor.shader_register_max;
			break;
		case shader_view_type_sampler:
			range_type = VK_DESCRIPTOR_TYPE_SAMPLER;
			if (sampler_shader_registers_count == 0) sampler_heap_start = sampler_heap_slot;
			sampler_shader_registers_count += descriptor.shader_register_max;
			sampler_heap_slot += descriptor.shader_register_max;
			break;
		default:
			throw std::exception("Resource type not supported %d", (int)descriptor.shader_view_type);
		}
		for (size_t j = 0; j < descriptor.shader_register_max; j++) {

			VkDescriptorSetLayoutBinding& binding = descriptor_ranges.emplace_back();
			binding.binding = bt++;
			binding.descriptorType = range_type;
			binding.descriptorCount = 1;
			binding.stageFlags = VK_SHADER_STAGE_ALL;
			binding.pImmutableSamplers = nullptr;
		}
	}

	//Descriptor Set Layout
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(descriptor_ranges.size());
	layoutInfo.pBindings = descriptor_ranges.data();

	VkDescriptorSetLayout descriptorSetLayout;
	if (vkCreateDescriptorSetLayout(*static_cast<VK_DEVICE*>(desc->device), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Error creating VkDescriptorSetLayout");
	}

	// Pipeline Layout (Root Signature global)
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

	// (Root Constants equ DX12) pushConstantRanges here
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	VkPipelineLayout pipelineLayout;
	if (vkCreatePipelineLayout(*static_cast<VK_DEVICE*>(desc->device), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Error creating VkPipelineLayout");
	}

	VK_PIPELINE_LAYOUT* result = new VK_PIPELINE_LAYOUT();
	result->set_handle(pipelineLayout);
	result->topology = desc->topology;
	result->surface_format = desc->surface_format;
	result->depth_buffer_format = desc->depth_buffer_format;
	result->constant_shader_registers_count = constant_shader_registers_count;
	result->sampler_shader_registers_count = sampler_shader_registers_count;
	result->read_only_shader_registers_count = read_only_shader_registers_count;
	result->rw_shader_registers_count = rw_shader_registers_count;
	result->read_only_texture_shader_registers_count = read_only_texture_shader_registers_count;
	result->rw_texture_shader_registers_count = rw_texture_shader_registers_count;
	result->constant_heap_start = constant_heap_start;
	result->sampler_heap_start = sampler_heap_start;
	result->read_only_heap_start = read_only_heap_start;
	result->rw_heap_start = rw_heap_start;
	result->read_only_texture_heap_start = read_only_texture_heap_start;
	result->rw_texture_heap_start = rw_texture_heap_start;
	return result;
}

RHI_PIPELINE_LAYOUT* vk_pipeline_layout_create2(const RHI_PIPELINE_LAYOUT_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	std::vector<VkDescriptorSetLayoutBinding> descriptor_ranges;
	descriptor_ranges.reserve(desc->descriptor_count);

	uint32_t bt = 0;
	size_t constant_shader_registers_count = 0;
	size_t sampler_shader_registers_count = 0;
	size_t read_only_shader_registers_count = 0;
	size_t rw_shader_registers_count = 0;
	size_t read_only_texture_shader_registers_count = 0;
	size_t rw_texture_shader_registers_count = 0;
	for (size_t i = 0; i < desc->descriptor_count; i++) {

		const RHI_SHADER_DESCRIPTOR_DESC& descriptor = desc->descriptors[i];
		VkDescriptorType range_type;
		switch (descriptor.shader_view_type) {
		case shader_view_type_read_only_texture_buffer:
			range_type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			read_only_texture_shader_registers_count += descriptor.shader_register_max;
			break;
		case shader_view_type_read_only_buffer:
			range_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			read_only_shader_registers_count += descriptor.shader_register_max;
			break;
		case shader_view_type_rw_texture_buffer:
			range_type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			rw_texture_shader_registers_count += descriptor.shader_register_max;
			break;
		case shader_view_type_rw_buffer:
			range_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			rw_shader_registers_count += descriptor.shader_register_max;
			break;
		case shader_view_type_constant_buffer:
			range_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			constant_shader_registers_count += descriptor.shader_register_max;
			break;
		case shader_view_type_sampler:
			range_type = VK_DESCRIPTOR_TYPE_SAMPLER;
			sampler_shader_registers_count += descriptor.shader_register_max;
			break;
		default:
			throw std::exception("Resource type not supported %d", (int)descriptor.shader_view_type);
		}
		for (size_t j = 0; j < descriptor.shader_register_max; j++) {

			VkDescriptorSetLayoutBinding& binding = descriptor_ranges.emplace_back();
			binding.binding = bt++;
			binding.descriptorType = range_type;
			binding.descriptorCount = 1;
			binding.stageFlags = VK_SHADER_STAGE_ALL;
			binding.pImmutableSamplers = nullptr;
		}
	}

	//Descriptor Set Layout
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(descriptor_ranges.size());
	layoutInfo.pBindings = descriptor_ranges.data();

	VkDescriptorSetLayout descriptorSetLayout;
	if (vkCreateDescriptorSetLayout(*static_cast<VK_DEVICE*>(desc->device), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Error creating VkDescriptorSetLayout");
	}

	// Pipeline Layout (Root Signature global)
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

	// (Root Constants equ DX12) pushConstantRanges here
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	VkPipelineLayout pipelineLayout;
	if (vkCreatePipelineLayout(*static_cast<VK_DEVICE*>(desc->device), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Error creating VkPipelineLayout");
	}

	VK_PIPELINE_LAYOUT* result = new VK_PIPELINE_LAYOUT();
	result->set_handle(pipelineLayout);
	result->topology = desc->topology;
	result->surface_format = desc->surface_format;
	result->depth_buffer_format = desc->depth_buffer_format;
	result->constant_shader_registers_count = constant_shader_registers_count;
	result->sampler_shader_registers_count = sampler_shader_registers_count;
	result->read_only_shader_registers_count = read_only_shader_registers_count;
	result->rw_shader_registers_count = rw_shader_registers_count;
	result->read_only_texture_shader_registers_count = read_only_texture_shader_registers_count;
	result->rw_texture_shader_registers_count = rw_texture_shader_registers_count;
	return result;
}
