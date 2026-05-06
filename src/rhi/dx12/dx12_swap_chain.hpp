#ifndef __dx12_swap_chain_hpp__
#define __dx12_swap_chain_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_SWAP_CHAIN> dx12_swap_chain_create(const RHI_SWAP_CHAIN_DESC& swpc_desc);
void dx12_swap_chain_present(RHI_SWAP_CHAIN& swap_chain);
std::shared_ptr<RHI_TEXTURE_2D> dx12_swap_chain_get_surface(RHI_SWAP_CHAIN& swap_chain, int surface_index = -1);
unsigned int dx12_swap_chain_get_current_buffer_id(RHI_SWAP_CHAIN& swap_chain);
#endif