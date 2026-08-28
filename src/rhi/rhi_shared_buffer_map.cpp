#include "rhi_shared_buffer_map.hpp"
#include "rhi_shared_buffer.hpp"

RhiSharedBufferMap::RhiSharedBufferMap(RhiSharedBuffer& buffer, size_t offset, size_t length)
	: m_buffer(&buffer)
	, m_offset(offset)
	, m_length(length)
	, m_data(static_cast<uint8_t*>(rhi_buffers_map_open(buffer, offset, length)))
	, m_mapped(true) {
}

RhiSharedBufferMap::~RhiSharedBufferMap() {

	if (m_mapped == true) {
		ASSERT_PTR(m_buffer);
		m_buffer->unmap(*this);
		m_mapped = false;
	}
}