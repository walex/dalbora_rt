#ifndef __dx12_pipeline_hpp__
#define __dx12_pipeline_hpp__

#include "dx12_hal.hpp"

std::unique_ptr<HAL_OBJECT> dx12_create_pipeline(const HAL_PIPELINE_DESC& pipeline_desc);

#endif