#include "dx12_swap_chain.hpp"
#include "dx12_factory.hpp"

std::unique_ptr<RHI_OBJECT> dx12_swap_chain_create(const RHI_SWAP_CHAIN_DESC& desc) {

	ID3D12Device* device = static_cast<DX_DEVICE_HANDLE&>(desc.device->get_native_handle());
	ID3D12CommandQueue* commandQueue = static_cast<DX_COMMAND_QUEUE_HANDLE&>(desc.command_queue->get_native_handle());
	IDXGIFactory5* factory = dx12_factory_get();
	HWND hwnd = desc.window->get_native_handle<RHI_WINDOW_HANDLE<HWND>&>();
	if (!device || !commandQueue || !factory) {
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
		auto hrTear = factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearSupported, sizeof(tearSupported));
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
	IDXGISwapChain1* swapChain1 = nullptr;
	HRESULT hr = factory->CreateSwapChainForHwnd(
		commandQueue,
		hwnd,
		&scDesc,
		nullptr, // fullscreen desc
		nullptr, // restrict to output
		&swapChain1
	);
	if (FAILED(hr) || !swapChain1) {
		if (swapChain1)
			swapChain1->Release();
		throw std::exception("Failed to create DXGI swap chain");
	}

	// Query for IDXGISwapChain3
	IDXGISwapChain3* swapChain3 = nullptr;
	hr = swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain3));
	if (FAILED(hr) || !swapChain3) {
		// Release the IDXGISwapChain1 (ComPtr will release) and throw
		throw std::exception("Failed to acquire IDXGISwapChain3");
	}

	return std::make_unique<RHI_OBJECT>(new DX_SWAP_CHAIN_HANDLE(swapChain3));
}

void dx12_swap_chain_present(RHI_OBJECT& swap_chain) {

	IDXGISwapChain1* h_swap_chain = static_cast<DX_SWAP_CHAIN_HANDLE&>(swap_chain.get_native_handle());
	h_swap_chain->Present(1, 0);
}