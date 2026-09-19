#ifndef __ResourceManager_hpp__
#define __ResourceManager_hpp__	

#include "Common.hpp"

class ResourceManager {

public:
	ResourceManager(RhiDevice& device);
	void create_descriptor_tables(const size_t constants_buffers_size,
		const size_t read_only_buffers_size, const size_t rw_buffers_size);
	const RhiDevice& get_device() const { return m_device; }
	RHI_MEMORY_DESCRIPTOR_SLOT* get_read_only_buffer_descriptor_slot();
	RHI_MEMORY_DESCRIPTOR_SLOT* get_constant_buffer_descriptor_slot();
	RHI_MEMORY_DESCRIPTOR_SLOT* get_rw_buffer_descriptor_slot();
	size_t get_constant_buffer_descriptor_size();
	size_t get_read_only_buffer_descriptor_size();
	size_t get_rw_buffer_descriptor_size();
	operator RhiMemoryTable& () const { return *m_buffers_memory_descriptors; }
	RhiMemoryTable* get_rtv_memory_descriptor() { return m_rtv_memory_descriptors.get(); }
	RhiMemoryTable* get_buffers_memory_descriptor() { return m_buffers_memory_descriptors.get(); }
private:
	std::unique_ptr<RhiMemoryTable> m_buffers_memory_descriptors;
	std::unique_ptr<RhiMemoryTable> m_rtv_memory_descriptors;
	RhiDevice& m_device;
	size_t m_constants_buffers_size = 0;
	size_t m_read_only_buffers_size = 0;
	size_t m_rw_buffers_size = 0;
};

#endif // !__ResourceManager_hpp__
