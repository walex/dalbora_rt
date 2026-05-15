#ifndef __dx12_rt_bvh_hpp__
#define __dx12_rt_bvh_hpp__

#include "dx12_rhi.hpp"

// https://developer.nvidia.com/rtx/raytracing/dxr/dx12-raytracing-tutorial-part-1
std::unique_ptr<RHI_RT_BVH> dx12_rt_bvh_create(const RHI_RT_BVH_DESC& desc);
void dx12_bvh_build_geometry_instances(const RT_GEOMETRY_INSTANCES_DESC& desc);

#endif