#ifndef __rhi_view_hpp__
#define __rhi_view_hpp__

#include "rhi_impl.hpp"

class RhiView 
	: public RhiImpl<RHI_VIEW> {

public:	
	RhiView(RHI_VIEW* handle = nullptr, size_t resource_id = 0);
	RhiView(RhiView&&) noexcept = default;
	RhiView& operator=(RhiView&&) noexcept = default;
	RhiView(const RhiView&) = delete;
	RhiView& operator=(const RhiView&) = delete;
	virtual ~RhiView() = default;	
	size_t get_view_id() const { return m_view_id; }
private:
	size_t m_view_id = -1;
};

#endif // __rhi_view_hpp__
