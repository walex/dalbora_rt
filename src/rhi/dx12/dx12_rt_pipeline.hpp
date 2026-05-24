#ifndef __dx12_rt_pipeline_hpp__
#define __dx12_rt_pipeline_hpp__

#include "dx12_rhi.hpp"

RHI_RT_PIPELINE* dx12_rt_pipeline_create(const RHI_RT_PIPELINE_DESC* const desc);
RHI_BUFFER* dx12_rt_pipeline_create_sbt(const RHI_DEVICE* const device, const RHI_RT_SBT_DESC* const desc, const RHI_RT_PIPELINE* const pipeline);

#endif