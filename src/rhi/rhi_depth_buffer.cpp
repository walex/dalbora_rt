#include "rhi_depth_buffer.hpp"
#include "rhi_device.hpp"
#include "rhi_command_buffer.hpp"
#include "rhi_shared_buffer.hpp"

RhiDepthBuffer::RhiDepthBuffer(RHI_BUFFER* handle)
	: RhiGPUBuffer(handle) {}

void RhiDepthBuffer::create(const RhiDevice& device, const size_t width, 
	const size_t height, resource_format format) {

	RHI_TEXTURE_2D_DESC desc;
	desc.device = device;
	desc.width = width;
	desc.height = height;
	desc.format = format;
	desc.type = buffer_type_depth_stencil;
	this->set_handle(rhi_buffers_create_depth(&desc));
}

void RhiDepthBuffer::upload(const RhiCommandBuffer& command_buffer, const RhiSharedBuffer& sb) {

	//rhi_buffers_gpu_upload(command_buffer, sb, *this);
}