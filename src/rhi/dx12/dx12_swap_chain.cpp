#include "dx12_swap_chain.hpp"
#include "dx12_factory.hpp"

std::unique_ptr<RHI_SWAP_CHAIN> dx12_swap_chain_create(const RHI_SWAP_CHAIN_DESC& desc) {

	ID3D12Device* i_device = static_cast<ID3D12Device*>(desc.device.get());
	ID3D12CommandQueue* i_command_queue = static_cast<ID3D12CommandQueue*>(desc.command_queue.get());
	IDXGIFactory5* i_factory = dx12_factory_get();
	HWND hwnd = static_cast<HWND>(static_cast<RHI_VOID_PTR>(desc.window.get()));
	if (!i_device || !i_command_queue || !i_factory) {
		throw std::exception("Invalid device/queue/factory for swapchain creation");
	}
	// Extract parameters from desc with sensible defaults if fields are missing
	UINT width = (UINT)((desc.width > 0) ? desc.width : 800);
	UINT height = (UINT)((desc.height > 0) ? desc.height : 600);
	UINT bufferCount = (UINT)((desc.buffer_count > 0) ? desc.buffer_count : 2);
	DXGI_FORMAT format = (desc.color_format != resource_format_none)
		?  dx12_resource_format_type[(int)desc.color_format]
		: DXGI_FORMAT_R8G8B8A8_UNORM;
	BOOL allowTearing = FALSE;

	// If tearing support requested/available, attempt to enable (best-effort)
	{
		BOOL tearSupported = FALSE;
		auto hrTear = i_factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearSupported, sizeof(tearSupported));
		if (SUCCEEDED(hrTear) && tearSupported) {
			allowTearing = desc.allow_tearing ? TRUE : FALSE;
		}
	}

	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = width;
	scDesc.Height = height;
	scDesc.Format = format;
	scDesc.Stereo = FALSE;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = bufferCount;
	scDesc.Scaling = DXGI_SCALING_STRETCH;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	scDesc.Flags = allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	// Create swap chain
	IDXGISwapChain1* i_swap_chain_1 = nullptr;
	HRESULT hr = i_factory->CreateSwapChainForHwnd(
		i_command_queue,
		hwnd,
		&scDesc,
		nullptr, // fullscreen desc
		nullptr, // restrict to output
		&i_swap_chain_1
	);
	if (FAILED(hr) || !i_swap_chain_1) {
		if (i_swap_chain_1)
			i_swap_chain_1->Release();
		throw std::exception("Failed to create DXGI swap chain");
	}

	// Query for IDXGISwapChain3
	IDXGISwapChain3* i_swap_chain_3 = nullptr;
	hr = i_swap_chain_1->QueryInterface(IID_PPV_ARGS(&i_swap_chain_3));
	i_swap_chain_1->Release();
	if (FAILED(hr) || !i_swap_chain_3) {
		if (i_swap_chain_3)
			i_swap_chain_3->Release();
		throw std::exception("Failed to acquire IDXGISwapChain3");
	}
	return std::make_unique<DX_SWAP_CHAIN>(i_swap_chain_3);
}

void dx12_swap_chain_present(RHI_SWAP_CHAIN& swap_chain) {

	static_cast<IDXGISwapChain3*>(swap_chain)->Present(1, 0);
}

std::shared_ptr<RHI_TEXTURE_2D> dx12_swap_chain_get_surface(RHI_SWAP_CHAIN& swap_chain, int surface_index) {

	ID3D12Resource* i_surface = nullptr;
	IDXGISwapChain3* i_swap_chain = static_cast<IDXGISwapChain3*>(swap_chain);
	if (surface_index < 0)
		surface_index = (int)i_swap_chain->GetCurrentBackBufferIndex();
	if (FAILED(i_swap_chain->GetBuffer(surface_index, IID_PPV_ARGS(&i_surface)))) {
		throw std::exception("Error getting surface");
	}
	D3D12_RESOURCE_DESC desc = i_surface->GetDesc();
	return std::make_shared<DX_TEXTURE_2D>(i_surface, static_cast<size_t>(desc.Width), static_cast<size_t>(desc.Height));
}

unsigned int dx12_swap_chain_get_current_buffer_id(RHI_SWAP_CHAIN& swap_chain) {

	return (unsigned int)static_cast<IDXGISwapChain3*>(swap_chain)->GetCurrentBackBufferIndex();
}