#ifndef __rhi_shared_buffer_map_hpp__
#define __rhi_shared_buffer_map_hpp__

#include "rhi_impl.hpp"

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
	bool m_mapped = false;
};

#endif