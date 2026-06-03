#ifndef __rhi_depth_buffer_hpp__
#define __rhi_depth_buffer_hpp__

#include "rhi_gpu_buffer.hpp"

class RhiDepthBuffer : public RhiGPUBuffer {
		
public:
	RhiDepthBuffer(RHI_BUFFER* handle = nullptr);
	virtual ~RhiDepthBuffer() = default;
	void create(const RhiDevice& device, const size_t width, 
		const size_t height, resource_format format);
	void upload(const RhiCommandBuffer& command_buffer, const RhiSharedBuffer& sb) override;
};

#endif // __rhi_depth_buffer_hpp__
