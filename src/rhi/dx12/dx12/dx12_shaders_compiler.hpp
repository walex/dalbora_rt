#ifndef __dx12_shaders_compiler_hpp__
#define __dx12_shaders_compiler_hpp__

#include "dx12_rhi.hpp"

void dx12_shaders_compiler_set_folder(const char* const folder);
std::unique_ptr<RHI_OBJECT> dx12_shaders_compiler_compile(const char* const file,
	const char* const entry, 
	const char* const target);

#endif