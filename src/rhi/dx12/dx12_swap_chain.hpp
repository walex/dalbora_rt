#ifndef __dx12_swap_chain_hpp__
#define __dx12_swap_chain_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_swap_chain_create(const RHI_SWAP_CHAIN_DESC& swpc_desc);
void dx12_swap_chain_present(RHI_OBJECT& swap_chain);

#endif