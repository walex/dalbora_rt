#ifndef __rhi_shared_buffer_hpp__
#define __rhi_shared_buffer_hpp__

#include "rhi_buffer.hpp"
#include "rhi_shared_buffer_map.hpp"
#include "rhi_view.hpp"

class RhiDevice;
class RhiView;
class RhiSharedBuffer: public RhiBuffer {

public:
	IMPLEMENT_MOVABLE_CLASS(RhiSharedBuffer);

	RhiSharedBuffer(RHI_BUFFER* handle = nullptr, buffer_memory_type = buffer_memory_type_default);
	virtual ~RhiSharedBuffer() = default;
	resource_format get_format();
	void create(const RhiDevice& device, const size_t length, const size_t stride = 0, const resource_format format = resource_format_none);
	RhiSharedBufferMap map(const size_t offset, const size_t length);
	void unmap(const RhiSharedBufferMap& map_info);
	void copy(const uint8_t* data, const size_t length, const size_t offset = 0);
	RhiView new_depth_buffer_view(const RhiDevice& device);
	RhiView new_constant_buffer_view(const RhiDevice& device);
	RhiView new_shader_read_only_view(const RhiDevice& device);
};

#endif // __rhi_shared_buffer_hpp__
