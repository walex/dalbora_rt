#include "rhi_descriptor_heap.hpp"
#include "rhi_device.hpp"

void RhiDescriptorHeap::create(const RhiDevice& device, const resource_type resource_type, 
	const size_t slot_count) {
    
	const RHI_DEVICE* device_impl = static_cast<const RHI_DEVICE*>(device);
	this->set_handle(rhi_descriptor_heap_create(device_impl, resource_type, slot_count, true));
	uint64_t cpu_descriptor_base_address = 0;
	uint64_t gpu_descriptor_base_address = 0;
	size_t descriptor_size = 0;
	rhi_descriptor_heap_get_info(*this, &cpu_descriptor_base_address, &gpu_descriptor_base_address, &descriptor_size);
	for (size_t i = 0; i < slot_count; ++i) {
		RhiDescriptorHeapResource& slot = m_slots.emplace_back(this);
		slot.slot_id = i;		
		slot.cpu_handle = cpu_descriptor_base_address + i * descriptor_size;
		slot.gpu_handle = gpu_descriptor_base_address + i * descriptor_size;
	}
}

RhiDescriptorHeapResource RhiDescriptorHeap::next_heap_slot() {
	if (m_slots.empty()) {
		throw std::exception("Descriptor heap is empty. Create the heap before requesting a slot id.");
	}
	RhiDescriptorHeapResource slot = m_slots.front();
	m_slots.pop_front();
	return slot;
}

void RhiDescriptorHeap::free_descriptor(RhiDescriptorHeapResource& slot) {
	m_slots.push_back(slot);
}