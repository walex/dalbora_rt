#include "dx12_raster_pipeline.hpp"

RHI_RASTER_PIPELINE* dx12_raster_pipeline_create(const RHI_RASTER_PIPELINE_DESC* const desc) {
	
	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);
	ASSERT_PTR(desc->layout);

	// For simplicity, we will create a basic graphics pipeline state object (PSO)
	ID3D12Device* i_device = *static_cast<DX_DEVICE*>(desc->device);
	ASSERT_PTR(i_device);
	constexpr D3D12_RASTERIZER_DESC rasterizer_desc_default = {
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_NONE,
		FALSE,
		D3D12_DEFAULT_DEPTH_BIAS,
		D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		TRUE,
		FALSE,
		FALSE,
		0,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	};


	constexpr D3D12_RENDER_TARGET_BLEND_DESC rt = {
		.BlendEnable = FALSE,
		.LogicOpEnable = FALSE,
		.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL
	};

	constexpr D3D12_BLEND_DESC belnd_desc_default = {
		.AlphaToCoverageEnable = FALSE,
		.IndependentBlendEnable = FALSE,
		.RenderTarget = { rt }
	};
	
	constexpr D3D12_DEPTH_STENCIL_DESC deep_stencil_desc_default = {
		TRUE,
		D3D12_DEPTH_WRITE_MASK_ALL,
		D3D12_COMPARISON_FUNC_LESS,
		FALSE,
		D3D12_DEFAULT_STENCIL_READ_MASK,
		D3D12_DEFAULT_STENCIL_WRITE_MASK,
		{},
		{},
	};

	D3D12_INPUT_LAYOUT_DESC input_layout;
	size_t element_count = desc->input_layouts_desc_count;
	std::vector<D3D12_INPUT_ELEMENT_DESC> layout_element_descs(element_count);
	input_layout.NumElements = static_cast<UINT>(element_count);
	for (int i = 0; i < element_count; i++) {
		D3D12_INPUT_ELEMENT_DESC& ele_desc = layout_element_descs.at(i);
		const RHI_INPUT_LAYOUT_DESC& gen_layout = desc->input_layouts_desc[i];
		ele_desc.SemanticName = &gen_layout.name[0];
		ele_desc.SemanticIndex = 0;
		ele_desc.Format = dx12_resource_format_type[(int)gen_layout.format];
		ele_desc.InputSlot = 0;
		ele_desc.AlignedByteOffset = gen_layout.offset;
		ele_desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		ele_desc.InstanceDataStepRate = 0;
	}
	input_layout.pInputElementDescs = &layout_element_descs[0];

	// Define a simple graphics pipeline state description
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = *static_cast<DX_PIPELINE_LAYOUT*>(desc->layout);
	if (desc->vertex_shader) {
		IDxcBlob* buffer = *static_cast<DX_COMPILED_SHADER_BUFFER*>(desc->vertex_shader);
		psoDesc.VS.pShaderBytecode = buffer->GetBufferPointer();
		psoDesc.VS.BytecodeLength = buffer->GetBufferSize();
	}
	if (desc->pixel_shader) {
		IDxcBlob* buffer = *static_cast<DX_COMPILED_SHADER_BUFFER*>(desc->pixel_shader);
		psoDesc.PS.pShaderBytecode = buffer->GetBufferPointer();
		psoDesc.PS.BytecodeLength = buffer->GetBufferSize();
	}
	psoDesc.BlendState = belnd_desc_default;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = rasterizer_desc_default;
	if (desc->depth_buffer_format != resource_format_none) {
		psoDesc.DepthStencilState = deep_stencil_desc_default;
		psoDesc.DSVFormat = dx12_resource_format_type[desc->depth_buffer_format];
	}
	psoDesc.InputLayout = input_layout;
	psoDesc.PrimitiveTopologyType = dx12_primitive_topology_type[desc->topology];
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = dx12_resource_format_type[(int)desc->surface_format];
	psoDesc.SampleDesc.Count = 1;

	ID3D12PipelineState* pipelineState = nullptr;
	ASSERT_SUCCESS(i_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
	ASSERT_PTR(pipelineState);

	DX_RASTER_PIPELINE* result = new DX_RASTER_PIPELINE();
	ASSERT_PTR(result);
	result->set_handle(pipelineState);
	result->layout = desc->layout;
	return result;
}