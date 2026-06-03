#include "dx12_swap_chain.hpp"
#include "dx12_factory.hpp"
#include "dx12_buffers.hpp"

RHI_SWAP_CHAIN* dx12_swap_chain_create(const RHI_SWAP_CHAIN_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);
	ASSERT_PTR(desc->command_queue);
	ASSERT_PTR(desc->window);
	ASSERT_PTR(desc->window->handle);

	DX_DEVICE* device_impl = static_cast<DX_DEVICE*>(desc->device);
	ID3D12Device* i_device = *device_impl;
	ASSERT_PTR(i_device);
	ASSERT_PTR(device_impl->rtv_heap.get());

	ID3D12CommandQueue* i_command_queue = *static_cast<DX_COMMAND_QUEUE*>(desc->command_queue);
	ASSERT_PTR(i_command_queue);

	IDXGIFactory5* i_factory = dx12_factory_get();
	ASSERT_PTR(i_factory);

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
	ASSERT_SUCCESS(i_factory->CreateSwapChainForHwnd(
		i_command_queue,
		hwnd,
		&scDesc,
		nullptr, // fullscreen desc
		nullptr, // restrict to output
		&i_swap_chain_1
	));
	ASSERT_PTR(i_swap_chain_1);

	// Query for IDXGISwapChain3
	IDXGISwapChain3* i_swap_chain_3 = nullptr;
	ASSERT_SUCCESS(i_swap_chain_1->QueryInterface(IID_PPV_ARGS(&i_swap_chain_3)));
	ASSERT_PTR(i_swap_chain_3);

	ID3D12DescriptorHeap* i_heap = *device_impl->rtv_heap.get();
	ASSERT_PTR(i_heap);
	DX_SWAP_CHAIN* swap_chain_impl = new DX_SWAP_CHAIN();
	swap_chain_impl->set_handle(i_swap_chain_3);

	for (UINT i = 0; i < bufferCount; i++) {
		ID3D12Resource* i_buffer;
		ASSERT_SUCCESS(i_swap_chain_3->GetBuffer(i, IID_PPV_ARGS(&i_buffer)));
		ASSERT_PTR(i_buffer);
	
		D3D12_RESOURCE_DESC texDesc = i_buffer->GetDesc();
		UINT16 mip_count = texDesc.MipLevels;
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
		for (UINT j = 0; j < mip_count; j++) {
			RHI_TEXTURE_MIPS& tm = mips.at(j);
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT& fp = layouts.at(j);
			tm.offset = static_cast<size_t>(fp.Offset);
			tm.num_rows = static_cast<size_t>(num_rows[j]);
			tm.pitch = static_cast<size_t>(fp.Footprint.RowPitch);
			tm.width = static_cast<size_t>(fp.Footprint.Width);
			tm.height = static_cast<size_t>(fp.Footprint.Height);
			tm.depth = static_cast<size_t>(fp.Footprint.Depth);
			tm.format = dx12_helpers_resource_format_from_dxgi_format(fp.Footprint.Format);
		}
		
		RHI_VIEW_DESC view_desc;
		view_desc.type = resource_type_render_target;
		view_desc.device = desc->device;
		view_desc.format = desc->color_format;
		view_desc.slot_id = i;
		std::unique_ptr<DX_BUFFER> buffer_wrapper
			= std::make_unique<DX_BUFFER>();
		buffer_wrapper->com_ptr.Attach(i_buffer);
		view_desc.buffer = buffer_wrapper.get();
		RHI_VIEW* view = dx12_buffers_create_view(&view_desc);
		buffer_wrapper->com_ptr.Detach();
		DX_TEXTURE_2D* texture = new DX_TEXTURE_2D();
		ASSERT_PTR(texture);
		i_buffer->AddRef();
		texture->set_handle(i_buffer);
		texture->format = desc->color_format;
		texture->width = desc->width;
		texture->height = desc->height;
		texture->length = static_cast<size_t>(totalUploadSize);
		texture->mip_maps_count = mip_count;
		memcpy(texture->mip_maps, mips.data(), sizeof(RHI_TEXTURE_MIPS) * mip_count);
		view->buffer = texture;

		swap_chain_impl->render_targets[swap_chain_impl->render_targets_count++].reset(view);
	}
	swap_chain_impl->format = desc->color_format;
	return swap_chain_impl;
}




void dx12_swap_chain_present(const RHI_SWAP_CHAIN* const swap_chain) {

	ASSERT_PTR(swap_chain);
	IDXGISwapChain3* i_swap_chain = *static_cast<const DX_SWAP_CHAIN*>(swap_chain);
	ASSERT_PTR(i_swap_chain);
	i_swap_chain->Present(1, 0);
}

RHI_VIEW* const dx12_swap_chain_get_surface(const RHI_SWAP_CHAIN* const swap_chain, 
	const size_t surface_index) {

	ASSERT_PTR(swap_chain);
	IDXGISwapChain3* i_swap_chain = *static_cast<const DX_SWAP_CHAIN*>(swap_chain);
	ASSERT_PTR(i_swap_chain);

	return swap_chain->render_targets[(surface_index >= swap_chain->render_targets_count)
		? i_swap_chain->GetCurrentBackBufferIndex()
		: surface_index].get();
}

uint32_t dx12_swap_chain_get_current_buffer_id(const RHI_SWAP_CHAIN* const swap_chain) {

	ASSERT_PTR(swap_chain);
	IDXGISwapChain3* i_swap_chain = *static_cast<const DX_SWAP_CHAIN*>(swap_chain);
	ASSERT_PTR(i_swap_chain);

	return static_cast<uint32_t>(i_swap_chain->GetCurrentBackBufferIndex());
}

