#ifndef __dx12_rt_bvh_hpp__
#define __dx12_rt_bvh_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_RESOURCE> dx12_rt_bvh_create(const RHI_RT_BVH_DESC& bvh_des);

#endif