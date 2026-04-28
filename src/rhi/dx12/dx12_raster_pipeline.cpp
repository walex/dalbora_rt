#include "dx12_raster_pipeline.hpp"

std::unique_ptr<RHI_OBJECT> dx12_create_raster_pipeline(const RHI_RASTER_PIPELINE_DESC& pipeline_desc) {
	// For simplicity, we will create a basic graphics pipeline state object (PSO)
	auto device = dx_rhi_get_interface<ID3D12Device>(*pipeline_desc.device);
	if (!device) {
		throw std::exception("Invalid device for pipeline creation");
	}
	D3D12_RASTERIZER_DESC rasterizer_desc_default = {
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

	D3D12_BLEND_DESC belnd_desc_default = {
		  FALSE,
		  FALSE,
	};

	D3D12_DEPTH_STENCIL_DESC deep_stencil_desc_default = {
		TRUE,
		D3D12_DEPTH_WRITE_MASK_ALL,
		D3D12_COMPARISON_FUNC_LESS,
		FALSE,
		D3D12_DEFAULT_STENCIL_READ_MASK,
		D3D12_DEFAULT_STENCIL_WRITE_MASK,
		{},
		{},
	};

	// Define a simple graphics pipeline state description
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = nullptr; // Assume root signature is set elsewhere
	psoDesc.VS = { nullptr, 0 }; // Vertex shader bytecode
	psoDesc.PS = { nullptr, 0 }; // Pixel shader bytecode
	psoDesc.BlendState = belnd_desc_default;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = rasterizer_desc_default;
	psoDesc.DepthStencilState = deep_stencil_desc_default;
	psoDesc.InputLayout = { nullptr, 0 }; // Input layout
	psoDesc.PrimitiveTopologyType = dx12_primitive_topology_type[(int)pipeline_desc.topology];
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	ID3D12PipelineState* pipelineState = nullptr;
	HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));
	if (FAILED(hr) || !pipelineState) {
		throw std::exception("Failed to create D3D12 graphics pipeline state");
	}
	return std::make_unique<RHI_OBJECT>(new DX_RASTER_PIPELINE_HANDLE(pipelineState));
}