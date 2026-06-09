#ifndef __dx12_rt_bvh_hpp__
#define __dx12_rt_bvh_hpp__

#include "dx12_rhi.hpp"

// https://developer.nvidia.com/rtx/raytracing/dxr/dx12-raytracing-tutorial-part-1
RHI_RT_BVH* dx12_rt_bvh_create(const RHI_RT_BVH_DESC* const desc);
RHI_BUFFER* dx12_rt_bvh_build_geometry_instances(const RHI_RT_BVH_GEOMETRY_DESC* const desc);
void dx12_rt_bvh_update_geometry_instances(const RHI_RT_BVH_GEOMETRY_DESC* const desc, RHI_BUFFER* const buffer);

#endif