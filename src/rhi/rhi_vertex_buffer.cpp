#include "rhi_vertex_buffer.hpp"
#include "rhi_device.hpp"

RhiVertexBuffer::RhiVertexBuffer(RHI_BUFFER* handle)
	: RhiImpl<RHI_BUFFER>(handle) {}

void RhiVertexBuffer::create(const RhiDevice& device, const size_t vertex_size, const size_t vertex_count, const resource_format format) {

	RHI_VERTEX_BUFFER_DESC desc;
	desc.device = device;
	desc.count = vertex_count;
	desc.length = vertex_count * vertex_size;
	desc.stride = vertex_size;
	desc.format = resource_format_float3;
	desc.memory_type = buffer_memory_type_gpu_only;
	desc.type = buffer_type_raw;
	this->set_handle(rhi_buffers_create_vertices(&desc));
}