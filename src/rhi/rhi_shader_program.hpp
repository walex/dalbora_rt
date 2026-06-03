#ifndef __rhi_shader_program_hpp__
#define __rhi_shader_program_hpp__

#include "rhi_impl.hpp"

class RhiShaderProgram
	: public ICreateRhiObject<const std::string&, const std::string&, const std::string&>
	, public RhiImpl<RHI_COMPILED_SHADER_BUFFER>{
	
public:
	RhiShaderProgram(RHI_COMPILED_SHADER_BUFFER* handle = nullptr);
	virtual ~RhiShaderProgram() = default;
	void create(const std::string& file_path, const std::string& entry_point, 
		const std::string& library_name);
	
};

#endif // __rhi_shader_program_hpp__