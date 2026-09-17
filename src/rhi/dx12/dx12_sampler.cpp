#include "dx12_sampler.hpp"
#include "dx12_heap.hpp"

RHI_SAMPLER* dx12_sampler_create(const RHI_SAMPLER_DESC* const desc) {

    ASSERT_PTR(desc);
    ASSERT_PTR(desc->device);
    ASSERT_PTR(desc->memory_descriptor);

    DX_DEVICE* device_impl = static_cast<DX_DEVICE*>(desc->device);
    ID3D12Device* i_device = *device_impl;
    ASSERT_PTR(i_device);

	D3D12_SAMPLER_DESC sampDesc = {};

    sampDesc.Filter =
        D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU =
        D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampDesc.AddressV =
        D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampDesc.AddressW =
        D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD =
        D3D12_FLOAT32_MAX;
    sampDesc.MipLODBias = 0.0f;
    sampDesc.MaxAnisotropy = 1;

    DX_SAMPLER* result = new DX_SAMPLER();
    ASSERT_PTR(result);

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle{static_cast<DX_MEMORY_DESCRIPTOR_SLOT*>(desc->memory_descriptor)->cpu_handle};
    i_device->CreateSampler(&sampDesc, cpu_handle);
    return result;
}