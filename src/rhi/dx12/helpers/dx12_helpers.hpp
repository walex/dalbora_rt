#ifndef __dx12_helpers_hpp__
#define __dx12_helpers_hpp__

#include "platform.hpp"
#include "rhi_c.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <wrl.h>

resource_format dx12_helpers_resource_format_from_dxgi_format(const DXGI_FORMAT format);
void dx12_helpers_copy_4x4Matrix_to_rt_instance(const float* const* transforms, const size_t instance_count,
    D3D12_GPU_VIRTUAL_ADDRESS gpu_mem, ID3D12Resource* i_buffer,
    size_t offset = 0);
#endif