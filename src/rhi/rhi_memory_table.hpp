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

class RhiMemory {};

class RhiDevice;
class RhiMemoryTable 
	: public RhiMemory
	, public ICreateRhiObject<const RhiDevice&, const memory_descriptor_type,
	const size_t>
	, public RhiImpl<RHI_MEMORY_DESCRIPTOR> {
public:
	IMPLEMENT_MOVABLE_CLASS(RhiMemoryTable);
	RhiMemoryTable(RHI_MEMORY_DESCRIPTOR* handle = nullptr) : RhiImpl<RHI_MEMORY_DESCRIPTOR>(handle) {}
	RhiMemoryTable(const RhiDevice& device, const memory_descriptor_type type,
		const size_t descriptors_count, std::vector<size_t> slot_group_group_offsets = {0}) : RhiImpl<RHI_MEMORY_DESCRIPTOR>(nullptr) { 
		this->create(device, type, descriptors_count);
		m_descriptor_group_offsets = slot_group_group_offsets;
	}
	std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT> next_descriptor(size_t group_index = 0) const;
private:

	void create(const RhiDevice& device, const memory_descriptor_type type,
		const size_t descriptors_count) override;

	mutable std::vector<size_t> m_descriptor_group_offsets = { 0 };
};

#endif