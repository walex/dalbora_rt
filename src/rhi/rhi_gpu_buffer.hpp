#ifndef __rhi_gpu_buffer_hpp__
#define __rhi_gpu_buffer_hpp__

#include "rhi_buffer.hpp"
#include "rhi_view.hpp"

class RhiDevice;
class RhiSharedBuffer;
class RhiCommandBuffer;
class RhiGPUBuffer: public RhiBuffer {
		

public:
	IMPLEMENT_COPYABLE_AND_MOVABLE_CLASS(RhiGPUBuffer);

	RhiGPUBuffer(RHI_BUFFER* handle = nullptr, buffer_memory_type type = buffer_memory_type_default);
	virtual ~RhiGPUBuffer() = default;
	virtual void create(const RhiDevice& device, const size_t length, 
		const size_t stride, const resource_format format = resource_format_none);
	virtual void upload(const RhiCommandBuffer& command_buffer, const RhiSharedBuffer& sb);
	virtual void upload(const RhiCommandBuffer& command_buffer, const RhiSharedBuffer& sb,
		const size_t ofsset_src, const size_t offset_dest, const size_t length);
	resource_format get_format();
	size_t get_length();
	size_t get_stride();
	RhiView new_depth_buffer_view(const RhiDevice& device);
	RhiView new_constant_buffer_view(const RhiDevice& device);
	RhiView new_shader_view(const RhiDevice& device);
};

#endif // __rhi_gpu_buffer_hpp__
