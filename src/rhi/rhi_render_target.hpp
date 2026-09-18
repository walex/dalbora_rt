#ifndef __rhi_render_target_hpp__
#define __rhi_render_target_hpp__

#include "rhi_texture.hpp"
#include "rhi_view.hpp"
#include "rhi_memory_table.hpp"
class RhiDevice;
class RhiMemoryTable;
class RhiRenderTarget
    : public RhiTexture
    , public ICreateRhiObject<const RhiDevice&, const resource_format,
                            const size_t, const size_t> {

public:
    RhiRenderTarget(RHI_TEXTURE_2D* handle = nullptr);
	virtual ~RhiRenderTarget() = default;
    void create(const RhiDevice& device, const resource_format format,
        const size_t width, const size_t heigh);
    RhiView new_view(const RhiDevice& device, RHI_MEMORY_DESCRIPTOR_SLOT* memory_descriptor_slot);
private:
    virtual RhiView new_read_only_view(const RhiDevice& device, RHI_MEMORY_DESCRIPTOR_SLOT* memory_descriptor_slot) override;
    virtual RhiView new_rw_view(const RhiDevice& device, RHI_MEMORY_DESCRIPTOR_SLOT* memory_descriptor_slotr) override  ;
};

#endif // __rhi_render_target_hpp__
