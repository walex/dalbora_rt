#include "rhi_shader_program.hpp"
#include "rhi_hlsl_compiler.hpp"
#include "rhi.hpp"

RhiShaderProgramHLSL::RhiShaderProgramHLSL(RHI_COMPILED_SHADER_BUFFER* handle)
	: RhiImpl<RHI_COMPILED_SHADER_BUFFER>(handle) {}

void RhiShaderProgramHLSL::create(const std::string& file_path,
	const std::string& entry_point, const std::string& library_name,
	byte_code_format format) {

	if (std::filesystem::exists(file_path) == false) {
		throw std::runtime_error("Shader file not found at path: " + file_path);
	}

	std::filesystem::path shader_file(file_path);
	std::string include_folder = shader_file.parent_path().string();//.append("include").string();

	RHI_COMPILED_SHADER_BUFFER* shader_buffer = rhi_hlsl_compiler_compile(file_path.c_str(), entry_point.c_str(), library_name.c_str(),
		include_folder.c_str(), format);
	this->set_handle(shader_buffer);
}

void RhiShaderProgramHLSL::from_bytecode(const uint8_t* const bytecode, const size_t size) {
	if (bytecode == nullptr || size == 0) {
		throw std::runtime_error("Invalid bytecode pointer or size");
	}
	RHI_COMPILED_SHADER_BUFFER* shader_buffer = new RHI_COMPILED_SHADER_BUFFER();
	shader_buffer->buffer = new uint8_t[size];
	memcpy(shader_buffer->buffer, bytecode, size);
	shader_buffer->buffer_size = size;
	this->set_handle(shader_buffer);
}

void RhiShaderProgramHLSL::from_binary_file(const std::string& file_path) {
	if (std::filesystem::exists(file_path) == false) {
		throw std::runtime_error("Shader binary file not found at path: " + file_path);
	}
	std::ifstream bytecode_file(file_path, std::ios::binary);
	if (!bytecode_file.is_open()) {
		throw std::runtime_error("Failed to open shader binary file: " + file_path);
	}
	bytecode_file.seekg(0, std::ios::end);
	size_t size = bytecode_file.tellg();
	bytecode_file.seekg(0, std::ios::beg);
	std::vector<uint8_t> bytecode(size);
	bytecode_file.read(reinterpret_cast<char*>(bytecode.data()), size);
	bytecode_file.close();
	this->from_bytecode(bytecode.data(), size);
}

void RhiShaderProgramHLSL::to_binary_file(const std::string& file_path) const {
	const RHI_COMPILED_SHADER_BUFFER* shader_buffer = static_cast<const RHI_COMPILED_SHADER_BUFFER*>(*this);
	if (shader_buffer == nullptr || shader_buffer->buffer == nullptr || shader_buffer->buffer_size == 0) {
		throw std::runtime_error("Shader buffer is empty or invalid");
	}
	std::ofstream bytecode_file(file_path, std::ios::binary);
	if (!bytecode_file.is_open()) {
		throw std::runtime_error("Failed to open file for writing: " + file_path);
	}
	bytecode_file.write(reinterpret_cast<const char*>(shader_buffer->buffer), shader_buffer->buffer_size);
	bytecode_file.close();
}