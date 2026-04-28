#ifndef __dx12_shaders_hpp__
#define __dx12_shaders_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_shaders_compile(const char* const file,
	const char* const entry, 
	const char* const target);

#endif