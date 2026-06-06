#include "rhi_render_pass.hpp"
#include "rhi.hpp"

RhiRenderPass::RhiRenderPass(RHI_RENDER_PASS* handle) : RhiImpl<RHI_RENDER_PASS>(handle) {}

void RhiRenderPass::set_render_target(RhiView& rt) {
	
	static_cast<RHI_RENDER_PASS*>(*this)->render_target_view = rt;
}

void RhiRenderPass::set_depth_buffer(RhiView& depth) {
	
	static_cast<RHI_RENDER_PASS*>(*this)->depth_buffer_view = depth;
}

void RhiRenderPass::set_view_port(const RHI_VIEWPORT& vp) {
	static_cast<RHI_RENDER_PASS*>(*this)->view_port = vp;
}

void RhiRenderPass::create(const RhiDevice& device) {

	RHI_RENDER_PASS_DESC desc;
	desc.device = device;
	this->set_handle(rhi_render_pass_create(&desc));
}

void RhiRasterRenderPass::render(RhiCommandBuffer& command_buffer, RhiRenderPassRenderCallback callback) {

	rhi_render_pass_execute_raster_mode(*this, command_buffer, [&] {
		callback(command_buffer);
		});
}

void RhiRayTraceRenderPass::render(RhiCommandBuffer& command_buffer, RhiRenderPassRenderCallback callback) {

	rhi_render_pass_execute_rt_mode(*this, command_buffer, [&] {
		callback(command_buffer);
		});
}
