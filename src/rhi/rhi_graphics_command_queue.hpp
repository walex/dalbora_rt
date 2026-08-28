#ifndef __rhi_graphics_command_queue_hpp__
#define __rhi_graphics_command_queue_hpp__

#include "rhi_command_queue.hpp"

class RhiGraphicsCommandQueue: public RhiCommandQueue {

public:
	IMPLEMENT_MOVABLE_CLASS(RhiGraphicsCommandQueue);
	RhiGraphicsCommandQueue(RHI_COMMAND_QUEUE* handle = nullptr);
	virtual ~RhiGraphicsCommandQueue() = default;
	void create(const RhiDevice& device) override;
};

#endif // __rhi_graphics_command_queue_hpp__