#include "rhi_shader_program.hpp"

RhiShaderProgram::RhiShaderProgram(RHI_COMPILED_SHADER_BUFFER* handle)
	: RhiImpl<RHI_COMPILED_SHADER_BUFFER>(handle) {}

void RhiShaderProgram::create(const std::string& file_path, const std::string& entry_point, 
	const std::string& library_name) {

	std::filesystem::path path(file_path);
	
	rhi_shaders_compiler_set_folder(path.parent_path().string().c_str());
	this->set_handle(rhi_shaders_compiler_compile(path.filename().string().c_str(), entry_point.c_str(),
		library_name.c_str()));
}
