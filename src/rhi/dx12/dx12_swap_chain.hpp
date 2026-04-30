#ifndef __dx12_swap_chain_hpp__
#define __dx12_swap_chain_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_swap_chain_create(const RHI_SWAP_CHAIN_DESC& swpc_desc);
void dx12_swap_chain_present(RHI_OBJECT& swap_chain);
std::unique_ptr<RHI_OBJECT> dx12_swap_chain_get_surface(RHI_OBJECT& swap_chain, int surface_index = -1);
unsigned int dx12_swap_chain_get_current_buffer_id(RHI_OBJECT& swap_chain);
#endif