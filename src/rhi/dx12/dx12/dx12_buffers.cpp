#include "dx12_buffers.hpp"
#include "dx12_resource_state.hpp"
#include "dx12_command_buffer.hpp"
#include "dx12_command_queue.hpp"

std::unique_ptr<RHI_OBJECT> dx12_buffers_create(const RHI_BUFFER_DESC& desc) {

	D3D12_RESOURCE_FLAGS  flags = D3D12_RESOURCE_FLAG_NONE;
	
	std::unique_ptr<D3D12_CLEAR_VALUE> clear_value;

	if (desc.format >= resource_format_d32_float_s8_uint
		&& desc.format <= resource_format_d16_norm) {

		flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		clear_value = std::make_unique<D3D12_CLEAR_VALUE>();
		clear_value->Format = dx12_resource_format_type[(int)desc.format];
		clear_value->DepthStencil.Depth = 1.0f;
		clear_value->DepthStencil.Stencil = 0;
	}
	else {
		for (auto f : buffer_resource_flags_type)
			flags |= (desc.flags & f) ? f : D3D12_RESOURCE_FLAG_NONE;
	}
	
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = dx12_heap_type[desc.memory_type];
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = dx12_buffer_type[(int)desc.type];
	bufferDesc.Alignment = 0;
	bufferDesc.Width = desc.width;
	bufferDesc.Height = desc.height;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	if (bufferDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	else
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	//bufferDesc.Layout = layout;
	bufferDesc.Flags = flags;
	bufferDesc.Format = (bufferDesc.Dimension > D3D12_RESOURCE_DIMENSION_BUFFER)
		? dx12_resource_format_type[(int)desc.format]
		: DXGI_FORMAT_UNKNOWN;
	ID3D12Resource* resource = nullptr;
	ID3D12Device* device = desc.device().handle<DX_DEVICE_HANDLE>();
	HRESULT hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		clear_value.get(),
		IID_PPV_ARGS(&resource)
	);
	if (FAILED(hr) || !resource) {
		throw std::exception("Failed to create D3D12 resource");
	}
	auto rhi_resource = std::make_unique<RHI_RESOURCE>(new DX_RESOURCE_HANDLE(resource));
	if (desc.initial_state != resource_state_none)
		dx12_resource_state_transition(desc.command_queue(), desc.command_buffer(), *rhi_resource, desc.initial_state);
	return rhi_resource;
}

std::unique_ptr<RHI_OBJECT> dx12_buffers_create_depth(const RHI_DEPTH_BUFFER_DESC& desc) {
	
	if (desc.pool == nullptr) {
		throw std::exception("Memory pool required.");
	}
	// overwrite desc to match must have depth buffer requeriments
	RHI_DEPTH_BUFFER_DESC db_desc_mutable = const_cast<RHI_DEPTH_BUFFER_DESC&>(desc);
	db_desc_mutable.memory_type = buffer_memory_type_default;
	db_desc_mutable.initial_state = resource_state_depth_write;
	db_desc_mutable.type = buffer_type_image_2d;
	if (db_desc_mutable.format < resource_format_d32_float_s8_uint
		|| db_desc_mutable.format > resource_format_d16_norm)
		throw std::exception("Invalid depth buffer format");
	auto depth_buffer_resournce = dx12_buffers_create(db_desc_mutable);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	ID3D12Resource* buffer_depth_buffer = depth_buffer_resournce->handle<DX_RESOURCE_HANDLE>();
	ID3D12Device* device = desc.device().handle<DX_DEVICE_HANDLE>();
	ID3D12DescriptorHeap* heap = desc.pool->handle<DX_DESCRIPTOR_POOL>();
	std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> dsvHandle = dx12_helpers_get_descriptor_heap_handle(device, heap, desc.slot);
	device->CreateDepthStencilView(buffer_depth_buffer, &dsvDesc, *dsvHandle);

	return nullptr;
}

void dx12_buffers_upload(RHI_TRANSFER_BUFFER_DESC& desc, const bool sync) {

	RHI_BUFFER_DESC upd_desc(desc.device, desc.command_queue, desc.command_buffer);
	upd_desc.width = desc.transfer_size;
	upd_desc.height = 1;
	upd_desc.memory_type = buffer_memory_type_cpu_to_gpu;
	auto& buffer_dest = dynamic_cast<RHI_RESOURCE&>(desc.buffer());
	ID3D12Resource* buffer_dest_h = buffer_dest.handle<DX_RESOURCE_HANDLE>();	
	auto buff = dx12_buffers_create(upd_desc);
	Microsoft::WRL::ComPtr<ID3D12Resource> buf_upd; 
	buf_upd.Attach(buff->handle<DX_RESOURCE_HANDLE>());
	ID3D12GraphicsCommandList* cmd_buffer = desc.command_buffer().handle<DX_COMMAND_BUFFER_HANDLE>();
	auto queue = desc.command_queue;
	void* mapped = nullptr;
	buf_upd->Map(0, nullptr, &mapped);
	memcpy(mapped, desc.data, desc.transfer_size);
	buf_upd->Unmap(0, nullptr);
	// set barrier
	auto current_state = buffer_dest.get_current_state();
	dx12_resource_state_transition(queue, desc.command_buffer(), desc.buffer, resource_state_copy_src);
	dx12_command_buffer_begin_record(desc.command_buffer());
	cmd_buffer->CopyBufferRegion(buffer_dest_h, 0, buf_upd.Get(), 0, desc.transfer_size);
	dx12_command_buffer_end_record(desc.command_buffer());
	dx12_command_queue_execute_synchronized(queue, desc.command_buffer());
	dx12_resource_state_transition(queue, desc.command_buffer(), desc.buffer, current_state);
}

void dx12_buffers_upload_synchronized(RHI_TRANSFER_BUFFER_DESC& desc) {
	dx12_buffers_upload(desc, true);
}

void dx12_buffers_download_synchronized(RHI_TRANSFER_BUFFER_DESC& desc) {

	RHI_BUFFER_DESC down_desc(desc.device, desc.command_queue, desc.command_buffer);
	down_desc.width = desc.transfer_size;
	down_desc.height = 1;
	down_desc.memory_type = buffer_memory_type_gpu_to_cpu;
	auto& buffer_src = reinterpret_cast<RHI_RESOURCE&>(desc.buffer);
	ID3D12Resource* buffer_src_h = buffer_src.handle<DX_RESOURCE_HANDLE>();
	ID3D12Resource* buf_dwnl = dx12_buffers_create(down_desc)->handle<DX_RESOURCE_HANDLE>();
	ID3D12GraphicsCommandList* cmd_buffer = desc.command_buffer().handle<DX_COMMAND_BUFFER_HANDLE>();
	auto queue = desc.command_queue;
	dx12_resource_state_transition(queue, desc.command_buffer(), desc.buffer, resource_state_copy_dest);
	cmd_buffer->CopyResource(buf_dwnl, buffer_src_h);
	auto current_state = buffer_src.get_current_state();
	dx12_resource_state_transition(queue, desc.command_buffer(), desc.buffer, current_state);
	dx12_command_queue_execute_synchronized(queue, desc.command_buffer());
	void* mapped = nullptr;
	D3D12_RANGE range = { 0, desc.transfer_size };
	buf_dwnl->Map(0, &range, &mapped);
	memcpy(desc.data, mapped, desc.transfer_size);
	buf_dwnl->Unmap(0, nullptr);
}