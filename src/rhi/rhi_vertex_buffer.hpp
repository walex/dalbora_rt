#ifndef __rhi_vertex_buffer_hpp__
#define __rhi_vertex_buffer_hpp__

#include "rhi_impl.hpp"

class RhiDevice;
class RhiVertexBuffer : public ICreateRhiObject<const RhiDevice&, const size_t, const size_t, const resource_format>
	, public RhiImpl<RHI_BUFFER> {
		

public:
	RhiVertexBuffer(RHI_BUFFER* handle = nullptr);
	virtual ~RhiVertexBuffer() = default;
	void create(const RhiDevice& device, const size_t vertex_size, const size_t vertex_count, const resource_format format);
	//void upload(const RhiSharedBuffer& buffer);
};

#endif // __rhi_vertex_buffer_hpp__
