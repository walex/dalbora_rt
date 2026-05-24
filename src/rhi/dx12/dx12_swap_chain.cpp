#include "dx12_swap_chain.hpp"
#include "dx12_factory.hpp"
#include "dx12_heap.hpp"

RHI_SWAP_CHAIN* dx12_swap_chain_create(const RHI_SWAP_CHAIN_DESC* const desc) {

	ASSERT_NULL(desc);
	ASSERT_NULL(desc->device);
	ASSERT_NULL(desc->command_queue);
	ASSERT_NULL(desc->window);
	ASSERT_NULL(desc->window->handle);

	DX_DEVICE* device_impl = static_cast<DX_DEVICE*>(desc->device);
	ID3D12Device* i_device = *device_impl;
	ASSERT_NULL(i_device);
	ASSERT_NULL(device_impl->rtv_heap.get());

	ID3D12CommandQueue* i_command_queue = *static_cast<DX_COMMAND_QUEUE*>(desc->command_queue);
	ASSERT_NULL(i_command_queue);

	IDXGIFactory5* i_factory = dx12_factory_get();
	ASSERT_NULL(i_factory);

	HWND hwnd = static_cast<HWND>(desc->window->handle);
	
	// Extract parameters from desc with sensible defaults if fields are missing
	UINT width = static_cast<UINT>((desc->width > 0) ? desc->width : 800);
	UINT height = static_cast<UINT>((desc->height > 0) ? desc->height : 600);
	UINT bufferCount = static_cast<UINT>((desc->buffer_count > 0) ? desc->buffer_count : 2);
	DXGI_FORMAT format = (desc->color_format != resource_format_none)
		?  dx12_resource_format_type[desc->color_format]
		: DXGI_FORMAT_R8G8B8A8_UNORM;
	BOOL allowTearing = FALSE;

	// If tearing support requested/available, attempt to enable (best-effort)
	BOOL tearSupported = FALSE;
	if (SUCCEEDED(i_factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearSupported, sizeof(tearSupported)))
		&& tearSupported) {
		allowTearing = desc->allow_tearing ? TRUE : FALSE;
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
	ASSERT_FAILED(i_factory->CreateSwapChainForHwnd(
		i_command_queue,
		hwnd,
		&scDesc,
		nullptr, // fullscreen desc
		nullptr, // restrict to output
		&i_swap_chain_1
	));
	ASSERT_NULL(i_swap_chain_1);

	// Query for IDXGISwapChain3
	IDXGISwapChain3* i_swap_chain_3 = nullptr;
	ASSERT_FAILED(i_swap_chain_1->QueryInterface(IID_PPV_ARGS(&i_swap_chain_3)));
	ASSERT_NULL(i_swap_chain_3);

	ID3D12DescriptorHeap* i_heap = *device_impl->rtv_heap.get();
	ASSERT_NULL(i_heap);
	DX_SWAP_CHAIN* swap_chain_impl = new DX_SWAP_CHAIN();
	swap_chain_impl->set_handle(i_swap_chain_3);

	for (size_t i = 0; i < bufferCount; i++) {
		ID3D12Resource* i_buffer;
		ASSERT_FAILED(i_swap_chain_3->GetBuffer(i, IID_PPV_ARGS(&i_buffer)));
		ASSERT_NULL(i_buffer);

		D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
		rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtv_desc.Format = dx12_resource_format_type[(int)desc->color_format];
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dx12_heap_next_handle(device_impl, heap_id_type_rtv);
		i_device->CreateRenderTargetView(i_buffer, &rtv_desc, rtvHandle);

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
		DX_VIEW* view = new DX_VIEW();
		ASSERT_NULL(view);
		DX_TEXTURE_2D* texture = new DX_TEXTURE_2D();
		ASSERT_NULL(texture);
		texture->set_handle(i_buffer);
		texture->format = desc->color_format;
		texture->width = desc->width;
		texture->height = desc->height;
		texture->length = static_cast<size_t>(totalUploadSize);
		memcpy(texture->mip_maps, mips.data(), sizeof(RHI_TEXTURE_MIPS) * mip_count);
		view->resource.reset(texture);

		swap_chain_impl->render_targets[swap_chain_impl->render_targets_count++].reset(view);
	}

	return swap_chain_impl;
}

void dx12_swap_chain_present(const RHI_SWAP_CHAIN* const swap_chain) {

	ASSERT_NULL(swap_chain);
	IDXGISwapChain3* i_swap_chain = *static_cast<const DX_SWAP_CHAIN*>(swap_chain);
	ASSERT_NULL(i_swap_chain);
	i_swap_chain->Present(1, 0);
}

const RHI_VIEW* const dx12_swap_chain_get_surface(const RHI_SWAP_CHAIN* const swap_chain, 
	const size_t surface_index) {

	ASSERT_NULL(swap_chain);
	ASSERT_EXPR(surface_index < swap_chain->render_targets_count);

	ASSERT_NULL(swap_chain);
	IDXGISwapChain3* i_swap_chain = *static_cast<const DX_SWAP_CHAIN*>(swap_chain);
	ASSERT_NULL(i_swap_chain);

	swap_chain->render_targets[(surface_index < 0)
		? i_swap_chain->GetCurrentBackBufferIndex()
		: surface_index];
}

uint32_t dx12_swap_chain_get_current_buffer_id(const RHI_SWAP_CHAIN* const swap_chain) {

	ASSERT_NULL(swap_chain);
	IDXGISwapChain3* i_swap_chain = *static_cast<const DX_SWAP_CHAIN*>(swap_chain);
	ASSERT_NULL(i_swap_chain);

	return static_cast<uint32_t>(i_swap_chain->GetCurrentBackBufferIndex());
}

