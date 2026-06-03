#include "rhi_command_queue.hpp"
#include "rhi_command_buffer.hpp"

RhiCommandQueueBufferList::RhiCommandQueueBufferList() {
	
	this->reserve(8);
}

void RhiCommandQueueBufferList::add_command_buffer(RhiCommandBuffer& command_buffer) {

	this->push_back(command_buffer);
}

RhiCommandQueue::RhiCommandQueue(RHI_COMMAND_QUEUE* handle) : RhiImpl<RHI_COMMAND_QUEUE>(handle) {}

void RhiCommandQueue::exec(RhiCommandQueueExecuteCallback callback) {

	rhi_command_queue_execute(*this, false, [&](
		RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
		std::vector<RHI_COMMAND_BUFFER*>* const command_buffer_list) {

			RhiCommandQueueBufferList list;
			callback(list);
			*command_buffer_list = std::move(list.get());
		});
}

void RhiCommandQueue::sync_exec(RhiCommandQueueExecuteCallback callback) {

	this->exec(callback);
	this->sync();
}

void RhiCommandQueue::sync() {
	rhi_command_queue_sync(*this);
}