#include "dx12_helpers.hpp"
#include "dx12_api_params.hpp"


std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> dx12_helpers_get_descriptor_heap_handle(ID3D12Device* device, ID3D12DescriptorHeap* heap, size_t slot) {

    D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
    if (slot + 1 > (int)desc.NumDescriptors) {
        throw std::exception("Max descriptors reached for type %d", desc.Type);
    }
    UINT rtvDescriptorSize =
        device->GetDescriptorHandleIncrementSize(desc.Type);
    auto h = heap->GetCPUDescriptorHandleForHeapStart();
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

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> 
dx12_helpers_create_descriptor_heap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, 
                                    size_t slot_count, D3D12_DESCRIPTOR_HEAP_FLAGS flags) {

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = type;
    heapDesc.NumDescriptors = static_cast<UINT>(slot_count);
    heapDesc.Flags = flags;
    heapDesc.NodeMask = 0;

    ID3D12DescriptorHeap* unifiedHeap = nullptr;
    auto hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&unifiedHeap));
    if (FAILED(hr)) {
        throw std::exception("Cannot create CBV_SRV_UAV descriptor heap");
    }
    return unifiedHeap;
}