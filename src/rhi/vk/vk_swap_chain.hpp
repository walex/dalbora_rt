#ifndef __vk_swap_chain_hpp__
#define __vk_swap_chain_hpp__

#include "vk_rhi.hpp"

RHI_SWAP_CHAIN* vk_swap_chain_create(const RHI_SWAP_CHAIN_DESC* const desc);
void vk_swap_chain_present(const RHI_SWAP_CHAIN* const swap_chain);
uint32_t vk_swap_chain_get_current_buffer_id(const RHI_SWAP_CHAIN* const swap_chain);
RHI_VIEW* vk_swap_chain_create_view(const RHI_DEVICE* const device, const RHI_SWAP_CHAIN* const swap_chain, const RHI_MEMORY_DESCRIPTOR_SLOT* memory_descriptor,
	resource_format format, const size_t buffer_id);


#endif