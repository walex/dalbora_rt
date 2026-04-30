#ifndef __dx12_helpers_hpp__
#define __dx12_helpers_hpp__

#include "platform.hpp"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <wrl.h>

Microsoft::WRL::ComPtr<ID3D12RootSignature> dx12_helpers_create_global_root_signature(ID3D12Device* device);
void dx12_create_RTV_heap_descriptors(ID3D12Device* device, int slot_count);
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dx12_helpers_get_heap_descriptor(D3D12_DESCRIPTOR_HEAP_TYPE type);
std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> dx12_helpers_get_descriptor_heap_handle(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, int slot = 0);

#endif