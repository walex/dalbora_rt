#ifndef __dx12_pipeline_hpp__
#define __dx12_pipeline_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_create_pipeline(const RHI_PIPELINE_DESC& pipeline_desc);

#endif