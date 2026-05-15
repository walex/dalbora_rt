#include "dx12_sampler.hpp"

std::unique_ptr<RHI_SAMPLER> dx12_sampler_create(const RHI_RT_SAMPLER_DESC& desc) {

    DX_DEVICE& device_impl = static_cast<DX_DEVICE&>(desc.device.get());
    ID3D12Device* i_device = device_impl;

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
   

    DX_HEAP* heap = device_impl.get_sampler_heap();
    auto sampler_handle = dx12_helpers_get_rw_descriptor_heap_handle(i_device, *heap, desc.resource_slot);
    i_device->CreateSampler(&sampDesc, *sampler_handle);
    return std::make_unique<DX_SAMPLER>(*sampler_handle);
}