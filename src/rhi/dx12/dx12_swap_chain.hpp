#ifndef __dx12_swap_chain_hpp__
#define __dx12_swap_chain_hpp__

#include "dx12_rhi.hpp"

RHI_SWAP_CHAIN* dx12_swap_chain_create(const RHI_SWAP_CHAIN_DESC* const desc);
void dx12_swap_chain_present(const RHI_SWAP_CHAIN* const swap_chain);
const RHI_TEXTURE_2D* const  dx12_swap_chain_get_surface(const RHI_SWAP_CHAIN* const swap_chain, size_t surface_index = -1);
uint32_t dx12_swap_chain_get_current_buffer_id(const RHI_SWAP_CHAIN* const swap_chain);

#endif