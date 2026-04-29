#ifndef __dx12_helpers_hpp__
#define __dx12_helpers_hpp__

#include "platform.hpp"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <wrl.h>

Microsoft::WRL::ComPtr<ID3D12RootSignature> dx12_helpers_create_global_root_signature(ID3D12Device* device);

#endif