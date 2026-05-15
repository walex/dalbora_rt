#ifndef __dx12_sampler_hpp__
#define __dx12_sampler_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_SAMPLER> dx12_sampler_create(const RHI_RT_SAMPLER_DESC& desc);

#endif