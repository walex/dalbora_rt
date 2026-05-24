#include "dx12_heap.hpp"

ID3D12DescriptorHeap*
dx12_heap_create_descriptor(const DX_DEVICE* const device_impl, const D3D12_DESCRIPTOR_HEAP_TYPE type,
	const size_t slot_count, const D3D12_DESCRIPTOR_HEAP_FLAGS flags) {

	ASSERT_NULL(device_impl);
	ID3D12Device* i_device = *device_impl;
	ASSERT_NULL(i_device);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = type;
	heapDesc.NumDescriptors = static_cast<UINT>(slot_count);
	heapDesc.Flags = flags;
	heapDesc.NodeMask = 0;

	ID3D12DescriptorHeap* i_heap = nullptr;
	ASSERT_FAILED(i_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&i_heap)));
	ASSERT_NULL(i_heap);
	return i_heap;
}

DX_HEAP* dx12_heap_create(const DX_DEVICE* const device_impl, resource_type resource_type, const size_t slot_count, const bool shader_visible) {

	ASSERT_NULL(device_impl);
	ID3D12Device* i_device = *device_impl;
	ASSERT_NULL(i_device);

	D3D12_DESCRIPTOR_HEAP_TYPE type;
	switch (resource_type) {
		case resource_type_generic_rw_buffer:
		case resource_type_constant_buffer:
		case resource_type_shader:
			type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			break;
		case resource_type_sampler:
			type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			break;
		case resource_type_render_target:
			type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			break;
		case resource_type_depth_stencil_target:
			type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			break;
	}	

	ID3D12DescriptorHeap* dh = dx12_heap_create_descriptor(device_impl, type,
		slot_count,
		(shader_visible == true)
		? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		: D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	DX_HEAP* result = new DX_HEAP();
	ASSERT_NULL(result);
	result->set_handle(dh);

	D3D12_DESCRIPTOR_HEAP_DESC desc = dh->GetDesc();
	result->count = 0;
	result->max_count = static_cast<size_t>(desc.NumDescriptors);
	result->descriptor_handle.cpu_descriptor_handle = dh->GetCPUDescriptorHandleForHeapStart();
	result->descriptor_handle.gpu_descriptor_handle = dh->GetGPUDescriptorHandleForHeapStart();
	result->descriptor_handle.descriptor_size = i_device->GetDescriptorHandleIncrementSize(desc.Type);

	return result;
}

size_t dx12_heap_next_handle(const DX_DEVICE* const device, 
	const heap_id_type heap_id,
	D3D12_CPU_DESCRIPTOR_HANDLE* const cpu_descriptor_handle,
	D3D12_GPU_DESCRIPTOR_HANDLE* const gpu_descriptor_handle) {

	ASSERT_NULL(device);
	ASSERT_EXPR(heap_id < heap_id_type_count);
	ASSERT_NULL(cpu_descriptor_handle);

	ID3D12Device* i_device = *device;
	ASSERT_NULL(i_device);

	DX_HEAP* heap_impl = nullptr;
	switch (heap_id) {
	case heap_id_type_resources:
		heap_impl = device->resources_heap.get();
		break;
	case heap_id_type_sampler:
		heap_impl = device->sampler_heap.get();
		break;
	case heap_id_type_rtv:
		heap_impl = device->rtv_heap.get();
		break;
	case heap_id_type_dsv:
		heap_impl = device->dsv_heap.get();
		break;
	}
	ASSERT_NULL(heap_impl);
	if (heap_impl->count < heap_impl->max_count)
		throw std::exception("Max descriptors reached for heap %d", heap_id);
	size_t& slot_id = heap_impl->count;
	*cpu_descriptor_handle = heap_impl->descriptor_handle.cpu_descriptor_handle;
	cpu_descriptor_handle->ptr += (slot_id++) * heap_impl->descriptor_handle.descriptor_size;

	if (gpu_descriptor_handle) {
		*gpu_descriptor_handle = heap_impl->descriptor_handle.gpu_descriptor_handle;
		gpu_descriptor_handle->ptr += (slot_id)*heap_impl->descriptor_handle.descriptor_size;
	}
	return heap_impl->descriptor_handle.descriptor_size;
}