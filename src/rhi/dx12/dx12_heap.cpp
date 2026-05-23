#include "dx12_heap.hpp"
#include "dx12_helpers.hpp"

// bindless root signature
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

	ID3D12DescriptorHeap* dh = dx12_helpers_create_descriptor_heap(device_impl, type,
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