#ifndef __dx12_heap_hpp__
#define __dx12_heap_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<DX_HEAP> dx12_heap_create(ID3D12Device* i_device, const DX_HEAP_DESC& desc);


#endif