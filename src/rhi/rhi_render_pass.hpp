#ifndef __rhi_render_pass_hpp__
#define __rhi_render_pass_hpp__

#include "rhi_impl.hpp"

class RhiCommandBuffer;
class RhiDevice;
class RhiTextureView;
class RhiView;
class RhiRasterPipeline;
using RhiRenderPassRenderCallback = std::function<void(RhiCommandBuffer&)>;
class RhiRenderPass 
	: public ICreateRhiObject<const RhiDevice&>
	,  RhiImpl<RHI_RENDER_PASS>{

public:
	RhiRenderPass(RHI_RENDER_PASS* handle = nullptr);
	virtual ~RhiRenderPass() = default;
	void create(const RhiDevice& device) override;
	void rasterize(RhiCommandBuffer& command_buffer, RhiRenderPassRenderCallback callback);
	void set_render_target(RhiView& rt);
	void set_depth_buffer(RhiView& depth);
	void set_pipeline(RhiRasterPipeline& pipeline);
	void set_view_port(const RHI_VIEWPORT& vp);
};

#endif // __rhi_render_pass_hpp__
