#include "dx12_heap.hpp"

ID3D12DescriptorHeap*
heap_create_dx12_descriptor(const DX_DEVICE* const device_impl, const D3D12_DESCRIPTOR_HEAP_TYPE type,
	const size_t slot_count, const D3D12_DESCRIPTOR_HEAP_FLAGS flags) {

	ASSERT_PTR(device_impl);
	ID3D12Device* i_device = *device_impl;
	ASSERT_PTR(i_device);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = type;
	heapDesc.NumDescriptors = static_cast<UINT>(slot_count);
	heapDesc.Flags = flags;
	heapDesc.NodeMask = 0;

	ID3D12DescriptorHeap* i_heap = nullptr;
	ASSERT_SUCCESS(i_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&i_heap)));
	ASSERT_PTR(i_heap);
	return i_heap;
}

RHI_MEMORY_DESCRIPTOR* heap_create_dx12_descriptor_table(const DX_DEVICE* const device_impl,
	const memory_descriptor_type heap_type,
	const size_t slots_size,
	const bool shader_visible) {

	ASSERT_PTR(device_impl);
	ID3D12Device* i_device = *device_impl;
	ASSERT_PTR(i_device);

	DX_MEMORY_DESCRIPTOR* result = new DX_MEMORY_DESCRIPTOR();
	ASSERT_PTR(result);

	D3D12_DESCRIPTOR_HEAP_TYPE type;
	switch (heap_type) {
	case memory_descriptor_type_buffer:
		type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		break;
	case memory_descriptor_type_sampler:
		type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		break;
	case memory_descriptor_type_dx_rtv:
		type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		const_cast<bool&>(shader_visible) = false; // RTV heap cannot be shader visible
		break;
	case memory_descriptor_type_dx_dsv:
		type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		const_cast<bool&>(shader_visible) = false; // DSV heap cannot be shader visible
		break;
	default:
		throw std::exception("heap type not supported");
	}
	ID3D12DescriptorHeap* dh = heap_create_dx12_descriptor(device_impl, type,
		slots_size,
		(shader_visible == true)
		? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		: D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	result->set_handle(dh);
	result->descriptor_count = slots_size;
	D3D12_DESCRIPTOR_HEAP_DESC desc = dh->GetDesc();
	result->cpu_handle = dh->GetCPUDescriptorHandleForHeapStart().ptr;
	if (shader_visible == true)
		result->gpu_handle = dh->GetGPUDescriptorHandleForHeapStart().ptr;
	else
		result->gpu_handle = 0;
	result->descriptor_size = i_device->GetDescriptorHandleIncrementSize(desc.Type);

	return result;
}

RHI_MEMORY_DESCRIPTOR* dx12_memory_resource_create(const RHI_MEMORY_RESOURCE_DESC* const desc) {
	
	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	switch (desc->type) {
	case memory_resource_type_descriptor_table:
		return heap_create_dx12_descriptor_table(static_cast<const DX_DEVICE*>(desc->device), desc->descriptor_desc.type,
			desc->descriptor_desc.count, desc->descriptor_desc.shader_visible);
	case memory_resource_type_pool:
		throw std::exception("memory resource type pool not implemented");
	default:
		throw std::exception("memory resource type not supported");
	}
}

RHI_MEMORY_DESCRIPTOR_SLOT* dx12_memory_resource_get_descriptor(const RHI_MEMORY_DESCRIPTOR* const heap, const size_t index) {
	
	ASSERT_EXPR(index < heap->descriptor_count);
	ASSERT_PTR(heap);
	const DX_MEMORY_DESCRIPTOR* dt = static_cast<const DX_MEMORY_DESCRIPTOR*>(heap);

	DX_MEMORY_DESCRIPTOR_SLOT* result = new DX_MEMORY_DESCRIPTOR_SLOT();
	ASSERT_PTR(result);

	result->slot_id = index;
	result->descriptor_size = dt->descriptor_size;
	result->descriptor_count = 1;
	if (dt->cpu_handle != 0) {
		result->cpu_handle = dt->cpu_handle + index * dt->descriptor_size;
	}
	else {
		result->cpu_handle = 0;
	}
	if (dt->gpu_handle != 0) {
		result->gpu_handle = dt->gpu_handle + index * dt->descriptor_size;
	}
	else {
		result->gpu_handle = 0;
	}
	return result;
}


