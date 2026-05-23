#include "dx12_helpers.hpp"
#include "dx12_api_params.hpp"
#include "dx12_rhi.hpp"

D3D12_CPU_DESCRIPTOR_HANDLE dx12_helpers_get_next_descriptor_heap_handle2(const DX_DEVICE* const device, const heap_id_type heap_id) {

    ASSERT_NULL(device);
    ASSERT_EXPR(heap_id < heap_id_type_count);

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
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle = heap_impl->descriptor_handle.cpu_descriptor_handle;;
    descriptor_handle.ptr += (slot_id++) * heap_impl->descriptor_handle.descriptor_size;
	return descriptor_handle;
}

//std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> dx12_helpers_get_rw_descriptor_heap_handle2(ID3D12Device* i_device, ID3D12DescriptorHeap* heap, size_t slot) {
//
//    D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
//    if (slot + 1 > (int)desc.NumDescriptors) {
//        throw std::exception("Max descriptors reached for type %d", desc.Type);
//    }
//    UINT rtvDescriptorSize =
//        i_device->GetDescriptorHandleIncrementSize(desc.Type);
//    auto h = heap->GetCPUDescriptorHandleForHeapStart();
//    h.ptr += (slot * rtvDescriptorSize);
//    return std::make_unique<D3D12_CPU_DESCRIPTOR_HANDLE>(h);
//}
//
//std::unique_ptr<D3D12_GPU_DESCRIPTOR_HANDLE> dx12_helpers_get_read_only_descriptor_heap_handle2(ID3D12Device* device, ID3D12DescriptorHeap* heap, size_t slot) {
//
//    D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
//    if (slot + 1 > (int)desc.NumDescriptors) {
//        throw std::exception("Max descriptors reached for type %d", desc.Type);
//    }
//    UINT rtvDescriptorSize =
//        device->GetDescriptorHandleIncrementSize(desc.Type);
//    auto h = heap->GetGPUDescriptorHandleForHeapStart();
//    h.ptr += (slot * rtvDescriptorSize);
//    return std::make_unique<D3D12_GPU_DESCRIPTOR_HANDLE>(h);
//}

ID3D12DescriptorHeap*
dx12_helpers_create_descriptor_heap(const DX_DEVICE* const device_impl, const D3D12_DESCRIPTOR_HEAP_TYPE type,
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

resource_format dx12_helpers_resource_format_from_dxgi_format(DXGI_FORMAT format) {

    std::span<const DXGI_FORMAT> s(dx12_resource_format_type);
    auto it = std::find(s.begin(), s.end(), format);
    if (it == s.end())
    {
        throw std::exception("texture format no supported");
    }
    return static_cast<resource_format>(std::distance(s.begin(), it));
}