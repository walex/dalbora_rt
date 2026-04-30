#include "dx12_buffers.hpp"
#include "dx12_resource_state.hpp"
#include "dx12_command_queue.hpp"

std::unique_ptr<RHI_OBJECT> dx12_buffers_create(const RHI_BUFFER_DESC& desc) {

	auto flags = desc.is_uav == true ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
	
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = dx12_heap_type[desc.memory_type];
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;
	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Alignment = 0;
	bufferDesc.Width = desc.size;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = flags;
	ID3D12Resource* resource;
	ID3D12Device* device = desc.device().handle<DX_DEVICE_HANDLE>();
	HRESULT hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		dx12_resource_state_type[(int)desc.initial_state],
		nullptr,
		IID_PPV_ARGS(&resource)
	);
	if (FAILED(hr) || !resource) {
		throw std::exception("Failed to create D3D12 resource");
	}
	return std::make_unique<RHI_RESOURCE>(new DX_RESOURCE_HANDLE(resource));
}

void dx12_buffers_upload(RHI_TRANSFER_BUFFER_DESC& desc, const bool sync) {

	RHI_BUFFER_DESC upd_desc(desc.device);
	upd_desc.size = desc.transfer_size;
	upd_desc.memory_type = buffer_memory_type_cpu_to_gpu;
	auto& buffer_dest = dynamic_cast<RHI_RESOURCE&>(desc.buffer());
	ID3D12Resource* buffer_dest_h = buffer_dest.handle<DX_RESOURCE_HANDLE>();
	ID3D12Resource* buf_upd = dx12_buffers_create(upd_desc)->handle<DX_RESOURCE_HANDLE>();
	ID3D12GraphicsCommandList* cmd_buffer = desc.command_buffer().handle<DX_COMMAND_BUFFER_HANDLE>();
	auto queue = desc.command_queue;
	void* mapped = nullptr;
	buf_upd->Map(0, nullptr, &mapped);
	memcpy(mapped, desc.data, desc.transfer_size);
	buf_upd->Unmap(0, nullptr);
	// set barrier
	auto current_state = buffer_dest.get_current_state();
	dx12_resource_state_transition(desc.command_buffer(), desc.buffer.get(), resource_state_copy_src);
	cmd_buffer->CopyBufferRegion(buffer_dest_h, 0, buf_upd, 0, desc.transfer_size);
	dx12_resource_state_transition(desc.command_buffer(), desc.buffer.get(), current_state);
	if (sync)
		dx12_command_queue_execute_synchronized(queue, desc.command_buffer());
}

void dx12_buffers_upload_synchronized(RHI_TRANSFER_BUFFER_DESC& desc) {
	dx12_buffers_upload(desc, true);
}

void dx12_buffers_download_synchronized(RHI_TRANSFER_BUFFER_DESC& desc) {

	RHI_BUFFER_DESC down_desc(desc.device);
	down_desc.size = desc.transfer_size;
	down_desc.memory_type = buffer_memory_type_gpu_to_cpu;
	auto& buffer_src = dynamic_cast<RHI_RESOURCE&>(desc.buffer.get());
	ID3D12Resource* buffer_src_h = buffer_src.handle<DX_RESOURCE_HANDLE>();
	ID3D12Resource* buf_dwnl = dx12_buffers_create(down_desc)->handle<DX_RESOURCE_HANDLE>();
	ID3D12GraphicsCommandList* cmd_buffer = desc.command_buffer().handle<DX_COMMAND_BUFFER_HANDLE>();
	auto queue = desc.command_queue;
	dx12_resource_state_transition(desc.command_buffer(), desc.buffer.get(), resource_state_copy_dest);
	cmd_buffer->CopyResource(buf_dwnl, buffer_src_h);
	auto current_state = buffer_src.get_current_state();
	dx12_resource_state_transition(desc.command_buffer(), desc.buffer.get(), current_state);
	dx12_command_queue_execute_synchronized(queue, desc.command_buffer());
	void* mapped = nullptr;
	D3D12_RANGE range = { 0, desc.transfer_size };
	buf_dwnl->Map(0, &range, &mapped);
	memcpy(desc.data, mapped, desc.transfer_size);
	buf_dwnl->Unmap(0, nullptr);
}