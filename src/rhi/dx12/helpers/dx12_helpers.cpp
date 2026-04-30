#include "dx12_helpers.hpp"
#include "dx12_api_params.h"

std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> g_heap_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES);
 
void dx12_create_heap_descriptors(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, int slot_count) {

    if (type >= D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES) {
        throw std::exception("Heap descriptor type not supported");
    }
    ID3D12DescriptorHeap* heap_descriptor;
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = slot_count;
    rtvHeapDesc.Type = type;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    auto hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&heap_descriptor));
    if (FAILED(hr)) {
        throw std::exception("Error creating RTV heap descriptors");
    }
    g_heap_descriptors[(int)type].Attach(heap_descriptor);
}

void dx12_create_RTV_heap_descriptors(ID3D12Device* device, int slot_count) {

    dx12_create_heap_descriptors(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, slot_count);
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dx12_helpers_get_heap_descriptor(D3D12_DESCRIPTOR_HEAP_TYPE type) {

    if (type >= D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES) {
        throw std::exception("Heap descriptor type not supported");
    }
    return g_heap_descriptors[(int)type];
}

std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> dx12_helpers_get_descriptor_heap_handle(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, int slot) {
    
    auto& descriptors = g_heap_descriptors[(int)type];
    D3D12_DESCRIPTOR_HEAP_DESC desc = descriptors->GetDesc();
    if (slot + 1 > desc.NumDescriptors) {
        throw std::exception("Max descriptors reached for type %d", type);
    }
    UINT rtvDescriptorSize =
        device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);  
    auto h = dx12_helpers_get_heap_descriptor(type)->GetCPUDescriptorHandleForHeapStart();
    h.ptr += (slot * rtvDescriptorSize);
    return std::make_unique<D3D12_CPU_DESCRIPTOR_HANDLE>(h);
}

Microsoft::WRL::ComPtr<ID3D12RootSignature> dx12_helpers_create_global_root_signature(ID3D12Device* device) {

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters = 0;
    desc.pParameters = nullptr;
    desc.NumStaticSamplers = 0;
    desc.pStaticSamplers = nullptr;
    desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

    Microsoft::WRL::ComPtr<ID3DBlob> serialized;
    Microsoft::WRL::ComPtr<ID3DBlob> error;

    HRESULT hr = D3D12SerializeRootSignature(
        &desc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &serialized,
        &error
    );

    if (FAILED(hr))
    {
        if (error)
            OutputDebugStringA((char*)error->GetBufferPointer());
        assert(false);
    }

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSig;
    device->CreateRootSignature(
        0,
        serialized->GetBufferPointer(),
        serialized->GetBufferSize(),
        IID_PPV_ARGS(&rootSig)
    );

    return rootSig;
}
