#include "rhi_graphics_command_queue.hpp"
#include "rhi.hpp"

RhiGraphicsCommandQueue::RhiGraphicsCommandQueue(RHI_COMMAND_QUEUE* handle) : RhiCommandQueue(handle) {}

void RhiGraphicsCommandQueue::create(const RhiDevice& device) {
	RHI_COMMAND_QUEUE_DESC desc;
	desc.device = device;
	this->set_handle(rhi_command_queue_create_for_render(&desc));
}