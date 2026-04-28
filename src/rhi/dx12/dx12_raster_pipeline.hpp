#ifndef __dx12_raster_pipeline_hpp__
#define __dx12_raster_pipeline_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_create_raster_pipeline(const RHI_RASTER_PIPELINE_DESC& pipeline_desc);

#endif