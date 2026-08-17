#include "rhi_view.hpp"
#include "rhi.hpp"

RhiView::RhiView(RHI_VIEW* handle, int resource_id) 
	: RhiImpl<RHI_VIEW>(handle)
	, m_view_id(resource_id) {}