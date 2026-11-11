#include "vk_raster_pipeline.hpp"

//#define VK_ALINED_SIZE(value, alignment)  (((value + (alignment - 1)) / alignment) * alignment)

RHI_RASTER_PIPELINE* vk_raster_pipeline_create(const RHI_RASTER_PIPELINE_DESC* const desc) {
    ASSERT_PTR(desc);
    ASSERT_PTR(desc->device);
    ASSERT_PTR(desc->layout);

    VkDevice i_device = *static_cast<VK_DEVICE*>(desc->device);

    VkGraphicsPipelineCreateInfo pipeline_info{};

    // ------------------------------------------------------------
    // Shader modules
    // ------------------------------------------------------------

    VkShaderModule vertex_shader_module = VK_NULL_HANDLE;
    VkShaderModule pixel_shader_module = VK_NULL_HANDLE;
	
    if (desc->vertex_shader)
    {
        VkShaderModuleCreateInfo create_info{};
        create_info.sType =
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

        create_info.codeSize =
            desc->vertex_shader->buffer_size;

        create_info.pCode =
            reinterpret_cast<const uint32_t*>(
                desc->vertex_shader->buffer);

        ASSERT_EXPR(vkCreateShaderModule(
            i_device,
            &create_info,
            nullptr,
            &vertex_shader_module) == VK_SUCCESS);
    }

    if (desc->pixel_shader)
    {
        VkShaderModuleCreateInfo create_info{};
        create_info.sType =
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

        create_info.codeSize =
            desc->pixel_shader->buffer_size;

        create_info.pCode =
            reinterpret_cast<const uint32_t*>(
                desc->pixel_shader->buffer);

        ASSERT_EXPR(vkCreateShaderModule(
            i_device,
            &create_info,
            nullptr,
            &pixel_shader_module) == VK_SUCCESS);
    }

    std::vector<VkDescriptorSetAndBindingMappingEXT> descriptor_set_and_binding_mapping;
    descriptor_set_and_binding_mapping.reserve(8);
    if (desc->resources_memory_descriptor.get()) {

        VkDescriptorSetAndBindingMappingEXT& stage_binding_mapping = descriptor_set_and_binding_mapping.emplace_back();
        stage_binding_mapping.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_AND_BINDING_MAPPING_EXT,
            stage_binding_mapping.descriptorSet = 0,
            stage_binding_mapping.firstBinding = 0,
            stage_binding_mapping.bindingCount = static_cast<uint32_t>(desc->layout->constant_shader_registers_count),
            stage_binding_mapping.resourceMask = VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT,
            stage_binding_mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT,
            stage_binding_mapping.sourceData = {
                    .constantOffset = {
                        .heapOffset = static_cast<uint32_t>(0),
                        .heapArrayStride = static_cast<uint32_t>(desc->resources_memory_descriptor->descriptor_size)
                    }
        };

    }

    std::unique_ptr<VkShaderDescriptorSetAndBindingMappingInfoEXT> descriptor_binding_mapping_info_ptr;
    if (descriptor_set_and_binding_mapping.size() > 0) {
		descriptor_binding_mapping_info_ptr = std::make_unique<VkShaderDescriptorSetAndBindingMappingInfoEXT>();
        descriptor_binding_mapping_info_ptr->sType = VK_STRUCTURE_TYPE_SHADER_DESCRIPTOR_SET_AND_BINDING_MAPPING_INFO_EXT;
        descriptor_binding_mapping_info_ptr->mappingCount = static_cast<uint32_t>(descriptor_set_and_binding_mapping.size());
        descriptor_binding_mapping_info_ptr->pMappings = descriptor_set_and_binding_mapping.data();
    }

    // ------------------------------------------------------------
    // Shader stages
    // ------------------------------------------------------------

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
	shader_stages.reserve(8);

    if (vertex_shader_module != VK_NULL_HANDLE)
    {
        VkPipelineShaderStageCreateInfo& stage =
			shader_stages.emplace_back();

        stage.sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        stage.stage =
            VK_SHADER_STAGE_VERTEX_BIT;

        stage.module =
            vertex_shader_module;

        stage.pName = desc->vertex_shader_entry_point;

        stage.pNext = descriptor_binding_mapping_info_ptr.get();
            
    }

    if (pixel_shader_module != VK_NULL_HANDLE)
    {
        VkPipelineShaderStageCreateInfo& stage =
			shader_stages.emplace_back();

        stage.sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        stage.stage =
            VK_SHADER_STAGE_FRAGMENT_BIT;

        stage.module =
            pixel_shader_module;

        stage.pName = desc->pixel_shader_entry_point;

        stage.pNext = descriptor_binding_mapping_info_ptr.get();
    }


    // ------------------------------------------------------------
    // Vertex input
    // ------------------------------------------------------------

    std::vector<VkVertexInputAttributeDescription>
        attribute_descriptions;

    attribute_descriptions.resize(
        desc->input_layouts_desc_count);

    for (uint32_t i = 0;
        i < desc->input_layouts_desc_count;
        ++i)
    {
        const RHI_INPUT_LAYOUT_DESC& src =
            desc->input_layouts_desc[i];

        VkVertexInputAttributeDescription& dst =
            attribute_descriptions[i];

        dst.location = i;
        dst.binding = 0;

        dst.format =
            vk_resource_format_type[
                static_cast<int>(src.format)];

        dst.offset =
            static_cast<uint32_t>(src.offset);
    }


    VkVertexInputBindingDescription binding{};

    binding.binding = 0;

    binding.stride = desc->vertex_stride;

    binding.inputRate =
        VK_VERTEX_INPUT_RATE_VERTEX;


    VkPipelineVertexInputStateCreateInfo vertex_input{};

    vertex_input.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    vertex_input.vertexBindingDescriptionCount = 1;
    vertex_input.pVertexBindingDescriptions = &binding;

    vertex_input.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(
            attribute_descriptions.size());

    vertex_input.pVertexAttributeDescriptions =
        attribute_descriptions.data();


    // ------------------------------------------------------------
    // Input assembly
    // ------------------------------------------------------------

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};

    input_assembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

    input_assembly.topology =
        vk_primitive_topology_type[
            static_cast<int>(desc->topology)];

    input_assembly.primitiveRestartEnable =
        VK_FALSE;


    // ------------------------------------------------------------
    // Viewport / Scissor
    // ------------------------------------------------------------

    VkPipelineViewportStateCreateInfo viewport_state{};

    viewport_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;


    // ------------------------------------------------------------
    // Rasterizer
    // ------------------------------------------------------------

    VkPipelineRasterizationStateCreateInfo rasterizer{};

    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

    rasterizer.depthClampEnable =
        VK_FALSE;

    rasterizer.rasterizerDiscardEnable =
        VK_FALSE;

    rasterizer.polygonMode =
        VK_POLYGON_MODE_FILL;

    // DX12:
    //
    // D3D12_CULL_MODE_NONE
    //
    rasterizer.cullMode =
        VK_CULL_MODE_NONE;

    rasterizer.frontFace =
        VK_FRONT_FACE_COUNTER_CLOCKWISE;

    rasterizer.depthBiasEnable =
        VK_FALSE;

    rasterizer.lineWidth =
        1.0f;


    // ------------------------------------------------------------
    // Multisampling
    // ------------------------------------------------------------

    VkPipelineMultisampleStateCreateInfo multisampling{};

    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

    multisampling.rasterizationSamples =
        VK_SAMPLE_COUNT_1_BIT;

    multisampling.sampleShadingEnable =
        VK_FALSE;


    // ------------------------------------------------------------
    // Depth / stencil
    // ------------------------------------------------------------

    VkPipelineDepthStencilStateCreateInfo depth_stencil{};

    depth_stencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    if (desc->depth_buffer_format != resource_format_none)
    {
        depth_stencil.depthTestEnable =
            VK_TRUE;

        depth_stencil.depthWriteEnable =
            VK_TRUE;

        depth_stencil.depthCompareOp =
            VK_COMPARE_OP_LESS;
    }
    else
    {
        depth_stencil.depthTestEnable =
            VK_FALSE;

        depth_stencil.depthWriteEnable =
            VK_FALSE;
    }

    depth_stencil.depthBoundsTestEnable =
        VK_FALSE;

    depth_stencil.stencilTestEnable =
        VK_FALSE;


    // ------------------------------------------------------------
    // Color blending
    // ------------------------------------------------------------

    VkPipelineColorBlendAttachmentState
        color_blend_attachment{};

    // DX12:
    //
    // BlendEnable = FALSE
    //
    color_blend_attachment.blendEnable =
        VK_FALSE;

    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;


    VkPipelineColorBlendStateCreateInfo color_blend{};

    color_blend.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

    color_blend.logicOpEnable =
        VK_FALSE;

    color_blend.attachmentCount =
        1;

    color_blend.pAttachments =
        &color_blend_attachment;


    // ------------------------------------------------------------
    // Dynamic state
    // ------------------------------------------------------------

    VkDynamicState dynamic_states[] =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state{};

    dynamic_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

    dynamic_state.dynamicStateCount =
        2;

    dynamic_state.pDynamicStates =
        dynamic_states;


    // ------------------------------------------------------------
    // Dynamic rendering
    // ------------------------------------------------------------

    VkPipelineRenderingCreateInfo rendering_info{};

    rendering_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

    VkFormat color_format =
        vk_resource_format_type[
            static_cast<int>(desc->format)];

    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachmentFormats =
        &color_format;

    if (desc->depth_buffer_format != resource_format_none)
    {
        rendering_info.depthAttachmentFormat =
            vk_resource_format_type[
                static_cast<int>(
                    desc->depth_buffer_format)];
    }


    // ------------------------------------------------------------
    // Graphics pipeline
    // ------------------------------------------------------------

   

    pipeline_info.sType =
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    pipeline_info.stageCount =
        static_cast<uint32_t>(shader_stages.size());

    pipeline_info.pStages =
        shader_stages.data();

    pipeline_info.pVertexInputState =
        &vertex_input;

    pipeline_info.pInputAssemblyState =
        &input_assembly;

    pipeline_info.pViewportState =
        &viewport_state;

    pipeline_info.pRasterizationState =
        &rasterizer;

    pipeline_info.pMultisampleState =
        &multisampling;

    pipeline_info.pDepthStencilState =
        &depth_stencil;

    pipeline_info.pColorBlendState =
        &color_blend;

    pipeline_info.pDynamicState =
        &dynamic_state;

    pipeline_info.renderPass =
        VK_NULL_HANDLE;

    pipeline_info.subpass =
        0;

    VkPipelineCreateFlags2CreateInfo pipeline_heap_flags{};
    if (descriptor_binding_mapping_info_ptr.get() != nullptr) {
        pipeline_heap_flags.sType = VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO;
        pipeline_heap_flags.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;
		rendering_info.pNext = &pipeline_heap_flags;
    }
    else {

        pipeline_info.layout =
            *static_cast<VK_PIPELINE_LAYOUT*>(
                desc->layout);
    }

    // Dynamic rendering
    pipeline_info.pNext =
        &rendering_info;


    // ------------------------------------------------------------
    // Create pipeline
    // ------------------------------------------------------------

    VkPipeline pipeline =
        VK_NULL_HANDLE;

    ASSERT_EXPR(vkCreateGraphicsPipelines(
        i_device,
        VK_NULL_HANDLE,
        1,
        &pipeline_info,
        nullptr,
        &pipeline) == VK_SUCCESS);


    // ------------------------------------------------------------
    // Destroy temporary shader modules
    // ------------------------------------------------------------

    if (vertex_shader_module != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(
            i_device,
            vertex_shader_module,
            nullptr);
    }

    if (pixel_shader_module != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(
            i_device,
            pixel_shader_module,
            nullptr);
    }


    // ------------------------------------------------------------
    // RHI object
    // ------------------------------------------------------------

    VK_RASTER_PIPELINE* result =
        new VK_RASTER_PIPELINE();

    ASSERT_PTR(result);

    result->set_handle(pipeline);

    result->layout =
        desc->layout;

    return result;
}