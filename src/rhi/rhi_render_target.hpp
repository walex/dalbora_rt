#ifndef __rhi_render_target_hpp__
#define __rhi_render_target_hpp__

#include "rhi_texture.hpp"
#include "rhi_view.hpp"

class RhiDevice;
class RhiRenderTarget
    : public RhiTexture
    , public ICreateRhiObject<const RhiDevice&, const resource_format,
                            const size_t, const size_t> {

public:
    RhiRenderTarget(RHI_TEXTURE_2D* handle = nullptr);
	virtual ~RhiRenderTarget() = default;
    void create(const RhiDevice& device, const resource_format format,
        const size_t width, const size_t heigh);
    RhiView new_rw_view(RhiDevice& device) {
        return RhiTexture::new_rw_view(device);
    }
};

#endif // __rhi_texture_hpp__
