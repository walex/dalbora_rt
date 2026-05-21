#ifndef __dx12_rt_pipeline_hpp__
#define __dx12_rt_pipeline_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_RT_PIPELINE> dx12_rt_pipeline_create(const RHI_RT_PIPELINE_DESC& pipeline_desc);
std::unique_ptr<RHI_BUFFER> dx12_rt_pipeline_create_sbt(RHI_DEVICE& device, RHI_RT_SBT_DESC& desc, RHI_RT_PIPELINE& pipeline);

#endif