#include "dx12_texture_2d.hpp"
#include "dx12_buffers.hpp"
#include "dx12_command_buffer.hpp"

RHI_TEXTURE_2D* dx12_texture_2d_create(const RHI_TEXTURE_2D_DESC* const desc)
{
	ASSERT_PTR(desc->device);
	DX_DEVICE* device_impl = static_cast<DX_DEVICE*>(desc->device);
	ID3D12Device *i_device = *device_impl;
	ASSERT_PTR(i_device);
	DX_HEAP* heap_impl = device_impl->resources_heap.get();
	ASSERT_PTR(heap_impl);
	ID3D12DescriptorHeap* i_heap = *heap_impl;
	ASSERT_PTR(i_heap);

	RHI_BUFFER_2D_DESC buff_desc = {};
	buff_desc.device = desc->device;
	buff_desc.format = desc->format;
	buff_desc.width = desc->width;
	buff_desc.height = desc->height;
	buff_desc.is_render_target = desc->is_render_target;
	buff_desc.mips = desc->mips;
	buff_desc.type = buffer_type_image_2d;
	std::unique_ptr<DX_BUFFER> buffer;
	buffer.reset(dx12_buffers_create_2d<DX_BUFFER>(&buff_desc));
	ASSERT_PTR(buffer.get());
	ID3D12Resource *i_texture = *static_cast<DX_BUFFER*>(*buffer);
	ASSERT_PTR(i_texture);
	i_texture->AddRef();


	// CREATE VIEW SEPARATELY
	//
	//std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> srv_handle = dx12_helpers_get_rw_descriptor_heap_handle(i_device, i_heap, heap_slot);

	//if (desc.default_state == resource_state_rt_render_target) {
	//	D3D12_UNORDERED_ACCESS_VIEW_DESC uav = {};

	//	uav.Format =
	//		dx12_resource_format_type[desc.format];

	//	uav.ViewDimension =
	//		D3D12_UAV_DIMENSION_TEXTURE2D;

	//	i_device->CreateUnorderedAccessView(
	//		i_texture,
	//		nullptr,
	//		&uav,
	//		*srv_handle
	//	);
	//}
	//else {
	//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//	srvDesc.Format = dx12_resource_format_type[desc.format];
	//	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//	srvDesc.Shader4ComponentMapping =
	//		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//	srvDesc.Texture2D.MipLevels = static_cast<UINT>(desc.mips);
	//	srvDesc.Texture1D.MostDetailedMip = 0;
	//	i_device->CreateShaderResourceView(
	//		i_texture,
	//		&srvDesc,
	//		*srv_handle);
	//}

	const UINT16 mip_count = static_cast<UINT16>(desc->mips);
	std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>
		layouts(mip_count);
	std::vector<UINT> num_rows(mip_count);
	std::vector<UINT64> row_sizes(mip_count);
	UINT64 totalUploadSize = 0;

	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = static_cast<UINT>(desc->width);
	texDesc.Height = static_cast<UINT>(desc->height);
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = mip_count;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	texDesc.Format = dx12_resource_format_type[(int)desc->format];

	i_device->GetCopyableFootprints(
		&texDesc,
		0,
		mip_count,
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

	DX_TEXTURE_2D* result = new DX_TEXTURE_2D();
	ASSERT_PTR(result);
	result->set_handle(i_texture);
	result->format = desc->format;
	result->width = desc->width;
	result->height = desc->height;
	result->length = static_cast<size_t>(totalUploadSize);
	memcpy(&result->mip_maps[0], mips.data(), sizeof(RHI_TEXTURE_MIPS) * mip_count);
	result->mip_maps_count = mip_count;
	return result;
}

void dx12_texture_2d_gpu_upload(RHI_COMMAND_BUFFER* const command_buffer,
	const RHI_BUFFER* const src_buffer,
	RHI_TEXTURE_2D* const dest_buffer) {

	ASSERT_PTR(command_buffer);
	ASSERT_PTR(src_buffer);
	ASSERT_PTR(dest_buffer);

	ID3D12GraphicsCommandList* i_command_buffer = *static_cast<DX_COMMAND_BUFFER*>(command_buffer);
	ASSERT_PTR(i_command_buffer);
	ID3D12Resource* i_texture = *static_cast<DX_TEXTURE_2D*>(dest_buffer);
	ASSERT_PTR(i_texture);

	static constexpr D3D12_RESOURCE_STATES resource_state[] = { D3D12_RESOURCE_STATE_COPY_DEST };
	static constexpr bool restore[] = { true };
	DX_RESOURCE* resources[] = { static_cast<DX_TEXTURE_2D*>(dest_buffer) };
	dx12_command_buffer_resource_transition(i_command_buffer,
		resources,
		resource_state,
		restore, 1, [&]() {

			for (UINT i = 0; i < static_cast<UINT>(dest_buffer->mip_maps_count); ++i)
			{
				auto& mip = dest_buffer->mip_maps[i];

				D3D12_TEXTURE_COPY_LOCATION src = {};
				src.pResource = *static_cast<const DX_BUFFER*>(src_buffer);
				src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				src.PlacedFootprint.Footprint.Format = dx12_resource_format_type[mip.format];
				src.PlacedFootprint.Footprint.Width = static_cast<UINT>(mip.width);
				src.PlacedFootprint.Footprint.Height = static_cast<UINT>(mip.height);
				src.PlacedFootprint.Footprint.Depth = static_cast<UINT>(mip.depth);
				src.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(mip.pitch);
				src.PlacedFootprint.Offset = static_cast<UINT64>(mip.offset);

				D3D12_TEXTURE_COPY_LOCATION dst = {};
				dst.pResource = i_texture;
				dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				dst.SubresourceIndex = i;

				i_command_buffer->CopyTextureRegion(
					&dst,
					0,
					0,
					0,
					&src,
					nullptr
				);
			}
		});
}