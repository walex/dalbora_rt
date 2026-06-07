#ifndef __rhi_shared_buffer_hpp__
#define __rhi_shared_buffer_hpp__

#include "rhi_buffer.hpp"
#include "rhi_view.hpp"

class RhiSharedBufferMap {

public:
	RhiSharedBufferMap(size_t offset, size_t length, uint8_t* data)
		: m_offset(offset)
		, m_length(length)
		, m_data(data) { }
	size_t get_offset() const { return m_offset; }
	size_t get_length() const { return m_length; }
	uint8_t* get_data() const { return m_data; }
private:
	size_t m_offset;
	size_t m_length;
	uint8_t* m_data;
};

class RhiDevice;
class RhiView;
class RhiSharedBuffer: public RhiBuffer {

public:
	IMPLEMENT_COPYABLE_AND_MOVABLE_CLASS(RhiSharedBuffer);

	RhiSharedBuffer(RHI_BUFFER* handle = nullptr);
	virtual ~RhiSharedBuffer() = default;
	void create(const RhiDevice& device, const size_t length, const size_t stride = 0, const resource_format format = resource_format_none);
	RhiSharedBufferMap map(const size_t offset, const size_t length);
	void unmap(const RhiSharedBufferMap& map_info);
	RhiView new_depth_buffer_view(RhiDevice& device);
	RhiView new_constant_buffer_view(RhiDevice& device);
};

#endif // __rhi_shared_buffer_hpp__
