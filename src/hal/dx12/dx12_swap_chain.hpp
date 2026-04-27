#ifndef __dx12_swap_chain_hpp__
#define __dx12_swap_chain_hpp__

#include "dx12_hal.hpp"

std::unique_ptr<HAL_OBJECT> dx12_create_swap_chain(const HAL_SWAP_CHAIN_DESC& swpc_desc);

#endif