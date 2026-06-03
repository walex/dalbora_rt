#ifndef __rhi_view_hpp__
#define __rhi_view_hpp__

#include "rhi_impl.hpp"

class RhiView 
	: public RhiImpl<RHI_VIEW> {

public:	
	virtual ~RhiView() = default;
protected:
	RhiView(RHI_VIEW* handle);
};

#endif // __rhi_view_hpp__
