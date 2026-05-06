#ifndef __dx12_pipeline_layout_hpp__
#define __dx12_pipeline_layout_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_pipeline_layout_create(const RHI_PIPELINE_LAYOUT_DESC& desc);


#endif