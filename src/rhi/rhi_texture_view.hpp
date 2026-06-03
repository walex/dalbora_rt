#ifndef __rhi_texture_view_hpp__
#define __rhi_texture_view_hpp__

#include "rhi_object.hpp"
#include "rhi_view.hpp"

class RhiTextureView : public RhiView {

public:
	RhiTextureView(RHI_VIEW* handle = nullptr);
	virtual ~RhiTextureView() = default;
};

#endif // __rhi_texture_view_hpp__
