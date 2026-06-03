#ifndef __rhi_view_hpp__
#define __rhi_view_hpp__

#include "rhi_impl.hpp"

class RhiView 
	: public RhiImpl<RHI_VIEW> {

public:	
	RhiView(RHI_VIEW* handle = nullptr);
	RhiView(RhiView&&) noexcept = default;
	RhiView& operator=(RhiView&&) noexcept = default;
	RhiView(const RhiView&) = delete;
	RhiView& operator=(const RhiView&) = delete;
	virtual ~RhiView() = default;	
};

class RhiDevice;
class IRhiViewCreator {
	virtual RhiView new_depth_buffer_view(RhiDevice& device) = 0;
	virtual RhiView new_constant_buffer_view(RhiDevice& device) = 0;
protected:
	IRhiViewCreator() = default;
};

#endif // __rhi_view_hpp__
