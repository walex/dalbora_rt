#include "dx12_raster_pipeline.hpp"

std::unique_ptr<RHI_OBJECT> dx12_raster_pipeline_create(const RHI_RASTER_PIPELINE_DESC& desc) {

	// For simplicity, we will create a basic graphics pipeline state object (PSO)
	ID3D12Device* device = desc.device().handle<DX_DEVICE_HANDLE>();
	if (!device) {
		throw std::exception("Invalid device for pipeline creation");
	}
	constexpr D3D12_RASTERIZER_DESC rasterizer_desc_default = {
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_BACK,
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
	std::vector<D3D12_INPUT_ELEMENT_DESC> layout_element_descs(desc.layouts.size());
	input_layout.NumElements = (UINT)desc.layouts.size();
	for (int i = 0; i < desc.layouts.size(); i++) {
		auto& ele_desc = layout_element_descs.at(i);
		auto& gen_layout = desc.layouts.at(i);
		ele_desc.SemanticName = gen_layout.name.c_str();
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
	psoDesc.pRootSignature = desc.pool().handle<DX_PIPELINE_LAYOUT_HANDLE>(); // Assume root signature is set elsewhere
	//psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	if (desc.vertex_shader) {
		IDxcBlob* buffer = desc.vertex_shader->handle<DX_SHADER_BUFFER_HANDLE>();
		psoDesc.VS.pShaderBytecode = buffer->GetBufferPointer();
		psoDesc.VS.BytecodeLength = buffer->GetBufferSize();
	}
	if (desc.pixel_shader) {
		IDxcBlob* buffer = desc.pixel_shader->handle<DX_SHADER_BUFFER_HANDLE>();
		psoDesc.PS.pShaderBytecode = buffer->GetBufferPointer();
		psoDesc.PS.BytecodeLength = buffer->GetBufferSize();
	}
//	psoDesc.BlendState = belnd_desc_default;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = rasterizer_desc_default;
	//psoDesc.DepthStencilState = deep_stencil_desc_default;
	psoDesc.InputLayout = input_layout;
	psoDesc.PrimitiveTopologyType = dx12_primitive_topology_type[(int)desc.topology];
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = dx12_resource_format_type[(int)desc.surface_format];
	psoDesc.SampleDesc.Count = 1;
	ID3D12PipelineState* pipelineState = nullptr;
	HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));
	if (FAILED(hr) || !pipelineState) {
		throw std::exception("Failed to create D3D12 graphics pipeline state");
	}
	return std::make_unique<RHI_OBJECT>(new DX_RASTER_PIPELINE_HANDLE(pipelineState));
}