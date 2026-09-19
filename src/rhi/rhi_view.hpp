#ifndef __rhi_view_hpp__
#define __rhi_view_hpp__

#include "rhi_impl.hpp"

class RhiCommandBuffer;
class RhiTexture;
class RhiDevice;
class RhiBuffer;
class RhiMemoryTable;
class RhiMemoryDescriptor;
class RhiView 
	: public RhiImpl<RHI_VIEW> 
	, public ICreateRhiObject<const RhiDevice&, const RhiBuffer&,
	const RhiMemoryDescriptor*, const shader_view_type,
	const resource_format, const size_t> {

public:	
	RhiView(RHI_VIEW* handle = nullptr, const bool ownership = false,
		RhiMemoryDescriptor* memory_descriptor_slot = nullptr);
	IMPLEMENT_MOVABLE_CLASS(RhiView);

	virtual ~RhiView() = default;	
	void create(const RhiDevice& device, const RhiBuffer& buffer,
		const RhiMemoryDescriptor* memory_descriptor_slot, const shader_view_type type,
		const resource_format format, const size_t mip_maps_count = 0);
	void blit(RhiCommandBuffer& command_buffer, RhiTexture& image);
	size_t get_descriptor_id() const { return static_cast<RHI_VIEW*>(*this)->memory_descriptor->slot_id; }
private:
	std::unique_ptr<const RhiMemoryDescriptor> m_memory_descriptor;
};

#endif // __rhi_view_hpp__
