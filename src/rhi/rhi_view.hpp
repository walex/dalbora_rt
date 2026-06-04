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

#endif // __rhi_view_hpp__
