#ifndef __rhi_render_pass_hpp__
#define __rhi_render_pass_hpp__

#include "rhi_impl.hpp"

class RhiCommandBuffer;
class RhiDevice;
class RhiTextureView;
class RhiView;
template<typename T>
class RhiPipeline;
using RhiRenderPassRenderCallback = std::function<void(RhiCommandBuffer&)>;

class RhiRenderPass 
	: public ICreateRhiObject<const RhiDevice&>
	, public RhiImpl<RHI_RENDER_PASS>{

public:
	virtual ~RhiRenderPass() = default;
	virtual void render(RhiCommandBuffer& command_buffer, RhiRenderPassRenderCallback callback) = 0;
	void create(const RhiDevice& device) override;
	void set_render_target(RhiView& rt);
	void set_depth_buffer(RhiView& depth);
	template<typename T>
	void set_pipeline(T& pipeline) { static_cast<RHI_RENDER_PASS*>(*this)->pipeline = pipeline; }
	void set_view_port(const RHI_VIEWPORT& vp);
protected:
	RhiRenderPass(RHI_RENDER_PASS* handle = nullptr);
};

class RhiRasterRenderPass
	: public RhiRenderPass {
public:
	void render(RhiCommandBuffer& command_buffer, RhiRenderPassRenderCallback callback);
};

class RhiRayTraceRenderPass
	: public RhiRenderPass {
public:
	void render(RhiCommandBuffer& command_buffer, RhiRenderPassRenderCallback callback);
};

#endif // __rhi_render_pass_hpp__
