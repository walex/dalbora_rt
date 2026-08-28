#ifndef __rhi_descriptor_heap_hpp__
#define __rhi_descriptor_heap_hpp__

#include "rhi_impl.hpp"

class RhiDescriptorHeap;
class RhiDescriptorHeapResource {
public:
	RhiDescriptorHeapResource() = default;
	RhiDescriptorHeapResource(RhiDescriptorHeap* heap) : heap(heap) {}
	~RhiDescriptorHeapResource() = default;
	
	uint64_t cpu_handle;
	uint64_t gpu_handle;
	size_t slot_id;
	RhiDescriptorHeap* heap;
};

class RhiDevice;
class RhiDescriptorHeap : public ICreateRhiObject<const RhiDevice&, const resource_type,
	const size_t>
	, public RhiImpl<RHI_HANDLE> {

public:
	IMPLEMENT_MOVABLE_CLASS(RhiDescriptorHeap);

	RhiDescriptorHeap(RHI_HANDLE* handle = nullptr) : RhiImpl<RHI_HANDLE>(handle) {}
	virtual ~RhiDescriptorHeap() = default;
	void create(const RhiDevice& device, const resource_type resource_type,
		const size_t slot_count);
	RhiDescriptorHeapResource next_heap_slot();
	void free_descriptor(RhiDescriptorHeapResource& slot);
private:
	std::deque<RhiDescriptorHeapResource> m_slots;
};

#endif // !__rhi_descriptor_heap_hpp__