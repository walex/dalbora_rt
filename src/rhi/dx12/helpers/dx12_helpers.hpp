#ifndef __dx12_helpers_hpp__
#define __dx12_helpers_hpp__

#define USE_DX12
#include "platform.hpp"
#undef USE_DX12

Microsoft::WRL::ComPtr<ID3D12RootSignature> dx12_helpers_create_global_root_signature(ID3D12Device* device);

#endif