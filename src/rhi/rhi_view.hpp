#ifndef __rhi_view_hpp__
#define __rhi_view_hpp__

#include "rhi_impl.hpp"
#include "rhi_descriptor_heap.hpp"

class RhiCommandBuffer;
class RhiTexture;
class RhiView 
	: public RhiImpl<RHI_VIEW> {

public:	
	RhiView(RHI_VIEW* handle, RhiDescriptorHeapResource&& slot);
	RhiView(RHI_VIEW* handle = nullptr, int resource_id = 0);

	IMPLEMENT_MOVABLE_CLASS(RhiView);

	virtual ~RhiView() = default;	
	int get_view_id() const { return m_view_id; }
	void blit(RhiCommandBuffer& command_buffer, RhiTexture& image);
private:
	int m_view_id = -1;
	RhiDescriptorHeapResource m_slot;
};

#endif // __rhi_view_hpp__
