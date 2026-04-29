#ifndef __dx12_rt_pipeline_hpp__
#define __dx12_rt_pipeline_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_rt_pipeline_create(const RHI_RT_PIPELINE_DESC& pipeline_desc);

#endif