#ifndef __vk_rt_pipeline_hpp__
#define __vk_rt_pipeline_hpp__

#include "vk_rhi.hpp"

RHI_RT_PIPELINE* vk_rt_pipeline_create(const RHI_RT_PIPELINE_DESC* const desc);
RHI_SBT_TABLE* vk_rt_pipeline_create_sbt(const RHI_DEVICE* const device, const RHI_RT_SBT_DESC* const desc, const RHI_RT_PIPELINE* const pipeline);

#endif