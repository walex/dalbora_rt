#ifndef __vk_rt_bvh_hpp__
#define __vk_rt_bvh_hpp__

#include "vk_rhi.hpp"

RHI_RT_BVH* vk_rt_bvh_create(const RHI_RT_BVH_DESC* const desc);
RHI_BUFFER* vk_rt_bvh_build_geometry_instances(const RHI_RT_BVH_GEOMETRY_DESC* const desc);
void vk_rt_bvh_update_geometry_instances(const RHI_RT_BVH_GEOMETRY_DESC* const desc, RHI_BUFFER* const buffer);

#endif