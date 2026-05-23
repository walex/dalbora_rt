#include "dx12_swap_chain.hpp"
#include "dx12_factory.hpp"

RHI_SWAP_CHAIN* dx12_swap_chain_create(const RHI_SWAP_CHAIN_DESC* const desc) {

	ASSERT_NULL(desc);

	DX_DEVICE& device_impl = static_cast<DX_DEVICE&>(desc->device.get());
	ID3D12Device* i_device = device_impl;
	ID3D12CommandQueue* i_command_queue = static_cast<DX_COMMAND_QUEUE&>(desc->command_queue.get());
	IDXGIFactory5* i_factory = dx12_factory_get();
	HWND hwnd = static_cast<HWND>(static_cast<RHI_VOID_PTR>(desc->window.get()));
	if (!i_device || !i_command_queue || !i_factory) {
		throw std::exception("Invalid device/queue/factory for swapchain creation");
	}
	// Extract parameters from desc with sensible defaults if fields are missing
	UINT width = (UINT)((desc->width > 0) ? desc->width : 800);
	UINT height = (UINT)((desc->height > 0) ? desc->height : 600);
	UINT bufferCount = (UINT)((desc->buffer_count > 0) ? desc->buffer_count : 2);
	DXGI_FORMAT format = (desc->color_format != resource_format_none)
		?  dx12_resource_format_type[(int)desc->color_format]
		: DXGI_FORMAT_R8G8B8A8_UNORM;
	BOOL allowTearing = FALSE;

	// If tearing support requested/available, attempt to enable (best-effort)
	{
		BOOL tearSupported = FALSE;
		auto hrTear = i_factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearSupported, sizeof(tearSupported));
		if (SUCCEEDED(hrTear) && tearSupported) {
			allowTearing = desc->allow_tearing ? TRUE : FALSE;
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

	DX_HEAP* heap_impl = device_impl.get_rtv_heap();
	if (heap_impl == nullptr) {
		throw std::exception("NO heap found for rtv.");
	}
	ID3D12DescriptorHeap* i_heap = static_cast<ID3D12DescriptorHeap*>(*heap_impl);
	DX_SWAP_CHAIN* swap_chain_impl = new DX_SWAP_CHAIN(i_swap_chain_3);

	for (size_t i = 0; i < bufferCount; i++) {
		ID3D12Resource* i_buffer;
		if (FAILED(i_swap_chain_3->GetBuffer(i, IID_PPV_ARGS(&i_buffer)))) {
			throw std::exception("unable to get swap chin buffer %d", i);
		}
		D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
		rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtv_desc.Format = dx12_resource_format_type[(int)desc->color_format];
		std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandle = dx12_helpers_get_rw_descriptor_heap_handle(i_device, i_heap, i);
		i_device->CreateRenderTargetView(i_buffer, &rtv_desc, *rtvHandle);

		DXGI_SWAP_CHAIN_DESC swp_desc;
		i_swap_chain_3->GetDesc(&swp_desc);
		UINT mip_count = 1;
		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = swp_desc.BufferDesc.Width;
		texDesc.Height = swp_desc.BufferDesc.Height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = mip_count;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		texDesc.Format = swp_desc.BufferDesc.Format;

		std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>
			layouts(mip_count);
		std::vector<UINT> num_rows(mip_count);
		std::vector<UINT64> row_sizes(mip_count);
		UINT64 totalUploadSize = 0;
		i_device->GetCopyableFootprints(
			&texDesc,
			0,
			1,
			0,
			layouts.data(),
			num_rows.data(),
			row_sizes.data(),
			&totalUploadSize);

		std::vector<RHI_TEXTURE_MIPS> mips(mip_count);
		for (UINT i = 0; i < mip_count; i++) {
			mips[i].offset = static_cast<size_t>(layouts[i].Offset);
			mips[i].num_rows = static_cast<size_t>(num_rows[i]);
			mips[i].pitch = static_cast<size_t>(layouts[i].Footprint.RowPitch);
			mips[i].width = static_cast<size_t>(layouts[i].Footprint.Width);
			mips[i].height = static_cast<size_t>(layouts[i].Footprint.Height);
			mips[i].depth = static_cast<size_t>(layouts[i].Footprint.Depth);
			mips[i].format = dx12_helpers_resource_format_from_dxgi_format(layouts[i].Footprint.Format);
		}

		swap_chain_impl->add_render_target(std::make_shared<DX_TEXTURE_2D>(i_buffer, *rtvHandle,
			resource_state_present, desc->color_format,
			desc->width, desc->height, static_cast<size_t>(totalUploadSize), std::move(mips)));
	}

	return swap_chain_impl;
}

void dx12_swap_chain_present(const RHI_SWAP_CHAIN* const swap_chain) {

	ASSERT_NULL(swap_chain);
	static_cast<IDXGISwapChain3*>(*const_cast<RHI_SWAP_CHAIN*>(swap_chain))->Present(1, 0);
}

const RHI_TEXTURE_2D* const dx12_swap_chain_get_surface(const RHI_SWAP_CHAIN* const swap_chain, size_t surface_index) {

	ASSERT_NULL(swap_chain);

	ID3D12Resource* i_surface = nullptr;
	IDXGISwapChain3* i_swap_chain = static_cast<IDXGISwapChain3*>(*const_cast<RHI_SWAP_CHAIN*>(swap_chain));
	if (surface_index < 0)
		surface_index = static_cast<uint32_t>(i_swap_chain->GetCurrentBackBufferIndex());
	return swap_chain->get_render_target(surface_index);
}

uint32_t dx12_swap_chain_get_current_buffer_id(const RHI_SWAP_CHAIN* const swap_chain) {

	return static_cast<uint32_t>(static_cast<IDXGISwapChain3*>(*const_cast<RHI_SWAP_CHAIN*>(swap_chain))->GetCurrentBackBufferIndex());
}

