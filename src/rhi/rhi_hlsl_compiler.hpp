#ifndef __rhi_hlsl_compiler_hpp__
#define __rhi_hlsl_compiler_hpp__

#include "rhi.hpp"


RHI_COMPILED_SHADER_BUFFER* rhi_hlsl_compiler_compile(const char* file_path,
	const char* entry_point,
	const char* target_lib,
	const char* include_folder,
	const byte_code_format format);
void rhi_hlsl_compiler_compile_to_file(const char* file_path,
	const char* entry_point,
	const char* target_lib,
	const char* include_folder,
	const byte_code_format format,
	const char* out_file_path);
#endif