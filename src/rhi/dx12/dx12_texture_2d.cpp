#include "dx12_texture_2d.hpp"
#include "dx12_buffers.hpp"

std::unique_ptr<RHI_TEXTURE_2D> dx12_texture_2d_create(const RHI_TEXTURE_2D_DESC &desc)
{
	DX_DEVICE &device_impl = reinterpret_cast<DX_DEVICE &>(desc.device.get());
	ID3D12Device *i_device = device_impl;
	DX_HEAP *heap_impl = device_impl.get_resources_heap();
	if (heap_impl == nullptr)
	{
		throw std::exception("NO heap found for dsv.");
	}

	std::unique_ptr<RHI_BUFFER> buffer = dx12_buffers_create_2d(desc);
	ID3D12Resource *i_texture = static_cast<DX_BUFFER &>(*buffer);
	i_texture->AddRef();

	
	size_t heap_slot = desc.resource_slot;

	ID3D12DescriptorHeap *i_heap = *heap_impl;
	std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> srv_handle = dx12_helpers_get_rw_descriptor_heap_handle(i_device, i_heap, heap_slot);

	if (desc.default_state == resource_state_rt_render_target) {
		D3D12_UNORDERED_ACCESS_VIEW_DESC uav = {};

		uav.Format =
			dx12_resource_format_type[desc.format];

		uav.ViewDimension =
			D3D12_UAV_DIMENSION_TEXTURE2D;

		i_device->CreateUnorderedAccessView(
			i_texture,
			nullptr,
			&uav,
			*srv_handle
		);
	}
	else {
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = dx12_resource_format_type[desc.format];
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping =
			D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = static_cast<UINT>(desc.mips);
		srvDesc.Texture1D.MostDetailedMip = 0;
		i_device->CreateShaderResourceView(
			i_texture,
			&srvDesc,
			*srv_handle);
	}

	const UINT mip_count = static_cast<UINT>(desc.mips);
	std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>
		layouts(mip_count);
	std::vector<UINT> num_rows(mip_count);
	std::vector<UINT64> row_sizes(mip_count);
	UINT64 totalUploadSize = 0;

	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = static_cast<UINT>(desc.width);
	texDesc.Height = static_cast<UINT>(desc.height);
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = mip_count;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	texDesc.Format = dx12_resource_format_type[(int)desc.format];

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

	return std::make_unique<DX_TEXTURE_2D>(i_texture, *srv_handle,
										   desc.default_state, desc.format,
										   static_cast<size_t>(desc.width), static_cast<size_t>(desc.height),
										   static_cast<size_t>(totalUploadSize), std::move(mips));
}

void dx12_texture_2d_gpu_upload(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER& cpu_buffer, RHI_TEXTURE_2D& texture) {

	ID3D12GraphicsCommandList* i_command_buffer = command_buffer;
	ID3D12Resource* i_texture = static_cast<DX_TEXTURE_2D&>(texture);

	resource_state old_state = texture.get_current_state();
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = i_texture;
	barrier.Transition.StateBefore = dx12_resource_state_type[old_state];
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	i_command_buffer->ResourceBarrier(1, &barrier);
	texture.set_current_state(resource_state_copy_dest);

	auto& mips = texture.get_mips();
	for (size_t i = 0; i < mips.size(); ++i)
	{
		auto& mip = texture.get_mips()[i];

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = static_cast<DX_BUFFER&>(cpu_buffer);
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint.Footprint.Format = dx12_resource_format_type[mip.format];
		src.PlacedFootprint.Footprint.Width = mip.width;
		src.PlacedFootprint.Footprint.Height = mip.height;
		src.PlacedFootprint.Footprint.Depth = mip.depth;
		src.PlacedFootprint.Footprint.RowPitch = mip.pitch;
		src.PlacedFootprint.Offset = mip.offset;

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

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = dx12_resource_state_type[old_state];
		i_command_buffer->ResourceBarrier(1, &barrier);
		texture.set_current_state(old_state);
	}

}