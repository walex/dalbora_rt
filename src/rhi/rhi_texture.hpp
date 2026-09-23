#ifndef __rhi_texture_hpp__
#define __rhi_texture_hpp__

#include "rhi_impl.hpp"
#include "rhi_view.hpp"

class RhiDevice;
class RhiSharedBuffer;
class RhiCommandBuffer;
class RhiMemoryTable;
class RhiTexture
    : public RhiImpl<RHI_TEXTURE_2D>
    , public ICreateRhiObject<const RhiDevice&, const resource_format,
                            const size_t, const size_t,
                            const bool, const size_t,
                            const size_t, const size_t> {

public:
	RhiTexture(RHI_TEXTURE_2D* handle = nullptr);
	virtual ~RhiTexture() = default;
    void create(const RhiDevice& device, const resource_format format,
        const size_t width, const size_t height,
        const bool is_cube_map, const size_t depth,
        const size_t dimension, const size_t mip_count);
    virtual RhiView new_view(const RhiDevice& device, const RhiMemoryDescriptor* memory_descriptor_slot);
    virtual RhiView new_read_only_view(const RhiDevice& device, const RhiMemoryDescriptor* memory_descriptor_slot);
    virtual RhiView new_rw_view(const RhiDevice& device, const RhiMemoryDescriptor* memory_descriptor_slot);
    size_t get_hw_length();
    const std::vector<RHI_TEXTURE_MIPS>& get_mips() const;
    void upload(RhiCommandBuffer& command_buffer, RhiSharedBuffer& buffer);
};

#endif // __rhi_texture_hpp__
