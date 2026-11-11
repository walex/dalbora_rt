#ifndef __rhi_shader_program_hpp__
#define __rhi_shader_program_hpp__

#include "rhi_impl.hpp"

class RhiShaderProgramHLSL
	: public ICreateRhiObject<const std::string&, const std::string&, 
	const std::string&, const byte_code_format>
	, public RhiImpl<RHI_COMPILED_SHADER_BUFFER>{
	
public:
	
	RhiShaderProgramHLSL(RHI_COMPILED_SHADER_BUFFER* handle = nullptr);
	virtual ~RhiShaderProgramHLSL() = default;
	void create(const std::string& file_path, const std::string& entry_point,
		const std::string& library_name, byte_code_format format = byte_code_format_input_hlsl);
	void from_bytecode(const uint8_t* const bytecode, const size_t size);
	void from_binary_file(const std::string& file_path);
	void to_binary_file(const std::string& file_path) const;

};

#endif // __rhi_shader_program_hpp__