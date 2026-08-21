#ifndef __rhi_shared_buffer_hpp__
#define __rhi_shared_buffer_hpp__

#include "rhi_buffer.hpp"
#include "rhi_view.hpp"

class RhiSharedBuffer;
class RhiSharedBufferMap {

public:
	IMPLEMENT_COPYABLE_AND_MOVABLE_CLASS(RhiSharedBufferMap);	

	RhiSharedBufferMap() = default;
	RhiSharedBufferMap(RhiSharedBuffer& buffer, size_t offset, size_t length);
	~RhiSharedBufferMap();
	size_t get_offset() const { return m_offset; }
	size_t get_length() const { return m_length; }
	uint8_t* get_data() const { return m_data; }
private:
	size_t m_offset;
	size_t m_length;
	uint8_t* m_data;
	RhiSharedBuffer* m_buffer;
};

class RhiDevice;
class RhiView;
class RhiSharedBuffer: public RhiBuffer {

public:
	IMPLEMENT_COPYABLE_AND_MOVABLE_CLASS(RhiSharedBuffer);

	RhiSharedBuffer(RHI_BUFFER* handle = nullptr, buffer_memory_type = buffer_memory_type_default);
	virtual ~RhiSharedBuffer() = default;
	void create(const RhiDevice& device, const size_t length, const size_t stride = 0, const resource_format format = resource_format_none);
	RhiSharedBufferMap map(const size_t offset, const size_t length);
	void unmap(const RhiSharedBufferMap& map_info);
	void copy(const uint8_t* data, const size_t length, const size_t offset = 0);
	RhiView new_depth_buffer_view(RhiDevice& device);
	RhiView new_constant_buffer_view(RhiDevice& device);
};

#endif // __rhi_shared_buffer_hpp__
