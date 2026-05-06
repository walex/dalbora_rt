#ifndef __dx12_helpers_hpp__
#define __dx12_helpers_hpp__

#include "platform.hpp"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <wrl.h>

Microsoft::WRL::ComPtr<ID3D12RootSignature> dx12_helpers_create_global_root_signature(ID3D12Device* device);
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dx12_helpers_create_descriptor_heap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, int slot_count, D3D12_DESCRIPTOR_HEAP_FLAGS flags);
std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> dx12_helpers_get_descriptor_heap_handle(ID3D12Device* device, ID3D12DescriptorHeap* heap, int slot);
#endif