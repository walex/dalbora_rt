#ifndef __dx12_swap_chain_hpp__
#define __dx12_swap_chain_hpp__

#include "dx12_rhi.hpp"

RHI_SWAP_CHAIN* dx12_swap_chain_create(const RHI_SWAP_CHAIN_DESC* const desc);
void dx12_swap_chain_present(const RHI_SWAP_CHAIN* const swap_chain);
uint32_t dx12_swap_chain_get_current_buffer_id(const RHI_SWAP_CHAIN* const swap_chain);
RHI_VIEW* dx12_swap_chain_create_view(const RHI_DEVICE* const device, const RHI_SWAP_CHAIN* const swap_chain, const RHI_MEMORY_DESCRIPTOR_SLOT* memory_descriptor,
	resource_format format, const size_t buffer_id);

#endif