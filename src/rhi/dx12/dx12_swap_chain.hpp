#ifndef __dx12_swap_chain_hpp__
#define __dx12_swap_chain_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_create_swap_chain(const RHI_SWAP_CHAIN_DESC& swpc_desc);

#endif