#ifndef __rhi_command_buffer_hpp__
#define __rhi_command_buffer_hpp__

#include "rhi_impl.hpp"

using RhiCommandBufferRecordCallback = std::function<void()>;

class RhiDevice;
class RhiCommandQueue;
class RhiGPUBuffer;
class RhiCommandBuffer 
	: public ICreateRhiObject<const RhiDevice&, const RhiCommandQueue&>
	, public RhiImpl<RHI_COMMAND_BUFFER>{

public:
	RhiCommandBuffer(RHI_COMMAND_BUFFER* handle = nullptr);
	virtual ~RhiCommandBuffer() = default;
	void create(const RhiDevice& device, const RhiCommandQueue& command_queue) override;
	void record(RhiCommandBufferRecordCallback callback);
	void draw_triangle_list(RhiGPUBuffer& vertex_buffer, RhiGPUBuffer* index_buffer = nullptr);
};

#endif // __rhi_command_buffer_hpp__