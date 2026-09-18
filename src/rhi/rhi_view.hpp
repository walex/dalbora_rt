#ifndef __rhi_view_hpp__
#define __rhi_view_hpp__

#include "rhi_impl.hpp"

class RhiCommandBuffer;
class RhiTexture;
class RhiDevice;
class RhiBuffer;
class RhiMemoryTable;
class RhiView 
	: public RhiImpl<RHI_VIEW> 
	, public ICreateRhiObject<const RhiDevice&, const RhiBuffer&,
	RHI_MEMORY_DESCRIPTOR_SLOT*, const shader_view_type,
	const resource_format, const size_t> {

public:	
	RhiView(RHI_VIEW* handle = nullptr, bool ownership = false);
	IMPLEMENT_MOVABLE_CLASS(RhiView);

	virtual ~RhiView() = default;	
	void create(const RhiDevice& device, const RhiBuffer& buffer,
		RHI_MEMORY_DESCRIPTOR_SLOT* memory_descriptor_slot, const shader_view_type type,
		const resource_format format, const size_t mip_maps_count = 0);
	void blit(RhiCommandBuffer& command_buffer, RhiTexture& image);
	size_t get_descriptor_id() const { return static_cast<RHI_VIEW*>(*this)->memory_descriptor->slot_id; }
private:
	std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT> m_memory_descriptor;
};

#endif // __rhi_view_hpp__
