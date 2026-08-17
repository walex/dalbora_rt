#ifndef __rhi_view_hpp__
#define __rhi_view_hpp__

#include "rhi_impl.hpp"

class RhiView 
	: public RhiImpl<RHI_VIEW> {

public:	
	RhiView(RHI_VIEW* handle = nullptr, int resource_id = 0);
	RhiView(RhiView&&) noexcept = default;
	RhiView& operator=(RhiView&&) noexcept = default;
	RhiView(const RhiView&) = delete;
	RhiView& operator=(const RhiView&) = delete;
	virtual ~RhiView() = default;	
	int get_view_id() const { return m_view_id; }
private:
	int m_view_id = -1;
};

#endif // __rhi_view_hpp__
