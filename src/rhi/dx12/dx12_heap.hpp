#ifndef __dx12_heap_hpp__
#define __dx12_heap_hpp__

#include "dx12_rhi.hpp"

DX_HEAP* dx12_heap_create(const DX_DEVICE* const device_impl, resource_type resource_type, const size_t slot_count, const bool shader_visible);
size_t dx12_heap_next_handle(const DX_DEVICE* const device,
	const heap_id_type heap_id,
	D3D12_CPU_DESCRIPTOR_HANDLE* const cpu_descriptor_handle,
	D3D12_GPU_DESCRIPTOR_HANDLE* const gpu_descriptor_handle = nullptr);
#endif