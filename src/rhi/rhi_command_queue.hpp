#ifndef __rhi_graphics_command_hpp__
#define __rhi_graphics_command_hpp__

#include "rhi_impl.hpp"

class RhiDevice;
class RhiCommandBuffer;
class RhiCommandQueueBufferList : private std::vector<RHI_COMMAND_BUFFER*> {
public:
	RhiCommandQueueBufferList();
	void add_command_buffer(RhiCommandBuffer& command_buffer);
	std::vector<RHI_COMMAND_BUFFER*>& get() { return *this; }
};
using RhiCommandQueueExecuteCallback = std::function<void(RhiCommandQueueBufferList&)>;

class RhiCommandQueue
	: public ICreateRhiObject<const RhiDevice&>
	, public RhiImpl<RHI_COMMAND_QUEUE> {

public:
	IMPLEMENT_MOVABLE_CLASS(RhiCommandQueue);
	virtual ~RhiCommandQueue() = default;
	void exec(RhiCommandQueueExecuteCallback callback);
	void sync_exec(RhiCommandQueueExecuteCallback callback);
	void sync();
protected:
	RhiCommandQueue(RHI_COMMAND_QUEUE* handle);
};

#endif // __rhi_command_queue_hpp__