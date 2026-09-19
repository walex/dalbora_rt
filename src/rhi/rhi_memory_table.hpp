#ifndef __rhi_memory_table_hpp__
#define __rhi_memory_table_hpp__

#include "rhi_impl.hpp"

// TODO REMOVE THIS
#define RESOURCES_HEAP_ENABLE true
#define BUFFERS_DESCRIPTORS_COUNT 1000

#define RTV_HEAP_ENABLE true
#define RTV_HEAP_DESCRIPTORS_COUNT 8

#define DSV_HEAP_ENABLE true
#define DSV_DESCRIPTORS_COUNT 1

#define SAMPLER_HEAP_ENABLE true
#define SAMPLER_DESCRIPTORS_COUNT 128

class RhiDevice;
class RhiMemoryDescriptor;
class RhiMemoryTable
	: public ICreateRhiObject<const RhiDevice&, const memory_descriptor_type,
	const size_t>
	, public RhiImpl<RHI_MEMORY_DESCRIPTOR> {
public:
	RhiMemoryTable(const RhiDevice& device, const memory_descriptor_type type,
		const size_t descriptors_count, const std::vector<size_t>& slot_group_group_offsets = {});
	virtual ~RhiMemoryTable();
	void next_descriptor_ptr(RHI_MEMORY_DESCRIPTOR_SLOT** slot, const size_t group_index = 0);
	RhiMemoryDescriptor* next_descriptor_ptr(size_t group_index = 0);
	std::unique_ptr<RhiMemoryDescriptor> next_descriptor(size_t group_index = 0);
	void restore_descriptor_slot(const size_t slot_id, const size_t group_index);
private:

	void create(const RhiDevice& device, const memory_descriptor_type type,
		const size_t descriptors_count) override;

	std::vector<std::queue<size_t>> m_descriptor_group_offsets;
	std::mutex m_descriptor_group_mutex;
};

class RhiMemoryDescriptor
	: public RhiImpl<RHI_MEMORY_DESCRIPTOR_SLOT> {
	
public:
	IMPLEMENT_MOVABLE_CLASS(RhiMemoryDescriptor);
	RhiMemoryDescriptor(RHI_MEMORY_DESCRIPTOR_SLOT* handle, RhiMemoryTable& table, size_t group_index) 
		: RhiImpl<RHI_MEMORY_DESCRIPTOR_SLOT>(handle, true), m_table(table), m_group_index(group_index) {}
	virtual ~RhiMemoryDescriptor() {
		m_table.restore_descriptor_slot(reinterpret_cast<RHI_MEMORY_DESCRIPTOR_SLOT*>(this)->slot_id, m_group_index);
	}
private:
	RhiMemoryTable& m_table;
	size_t m_group_index;
};

#endif