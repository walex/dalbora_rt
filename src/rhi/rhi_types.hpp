#ifndef __rhi_types_hpp__
#define __rhi_types_hpp__

#include "rhi_defs.hpp"
#include <Eigen/Dense>

struct RHI_DEVICE {
	RHI_STRUCT_BASE_INFO(RHI_DEVICE, () {})
};

struct RHI_SWAP_CHAIN {
	RHI_STRUCT_BASE_INFO(RHI_SWAP_CHAIN, () {})
};

struct RHI_FENCE {
	RHI_STRUCT_BASE_INFO(RHI_FENCE, () {})
};

struct RHI_COMPILED_SHADER_BUFFER {
	RHI_STRUCT_BASE_INFO(RHI_COMPILED_SHADER_BUFFER, () {})
};

struct RHI_PIPELINE_LAYOUT {
	RHI_STRUCT_BASE_INFO(RHI_PIPELINE_LAYOUT, () {})
};

struct RHI_COMMAND_BUFFER {
	RHI_STRUCT_BASE_INFO(RHI_COMMAND_BUFFER, () {})
};

struct RHI_GRAPHICS_PIPELINE {
	RHI_STRUCT_BASE_INFO(RHI_GRAPHICS_PIPELINE, RHI_STRUCT_BASE_PARAMS(

		(pipeline_type type) : type(type) {}
	))
	pipeline_type get_type() { return type; }
private:
	pipeline_type type;
};

struct RHI_RASTER_PIPELINE : public RHI_GRAPHICS_PIPELINE {
	RHI_RASTER_PIPELINE(RHI_PIPELINE_LAYOUT& layout)
		: RHI_GRAPHICS_PIPELINE(pipeline_type_raster)
		, layout(layout) {}
	operator RHI_PIPELINE_LAYOUT& () {return layout.get(); }
private:
	std::reference_wrapper<RHI_PIPELINE_LAYOUT> layout;
};

struct RHI_RT_PIPELINE : public RHI_GRAPHICS_PIPELINE {
	RHI_RT_PIPELINE() : RHI_GRAPHICS_PIPELINE(pipeline_type_rt) {}
};

struct RHI_COMMAND_QUEUE {
	RHI_STRUCT_BASE_INFO(RHI_COMMAND_QUEUE, RHI_STRUCT_BASE_PARAMS(

		(std::unique_ptr<RHI_FENCE>&& fence)
		: m_fence(std::move(fence))
		, m_fence_counter(0) {
	}
	))
	RHI_FENCE& get_fence() { return *m_fence.get(); }
	unsigned __int64 increment_fence_counter() { return ++m_fence_counter; }
	unsigned __int64 get_fence_counter() { return m_fence_counter; }
private:
	std::unique_ptr<RHI_FENCE> m_fence;
	unsigned __int64 m_fence_counter;
};

struct RHI_RESOURCE {
	RHI_STRUCT_BASE_INFO(RHI_RESOURCE, RHI_STRUCT_BASE_PARAMS(

		(resource_state base_state, resource_format resource_format)
		: current_state(resource_state_none), base_state(base_state), format(resource_format) {
	}
	))
public:
	DEFINE_SETTER(current_state);
	DEFINE_GETTER(current_state);
	DEFINE_GETTER(base_state);
	DEFINE_GETTER(format);
private:
	resource_state current_state;
	resource_state base_state;
	resource_format format;
};

struct RHI_BUFFER: public RHI_RESOURCE {

	RHI_STRUCT_BASE_INFO(RHI_BUFFER, RHI_STRUCT_BASE_PARAMS(

		(resource_state base_state, resource_format resource_format, size_t width, size_t height, size_t stride)
		: RHI_RESOURCE(base_state, resource_format)
		, width(width)
		, height(height)
		, stride(stride) {
	}
	))
public:
	DEFINE_SETTER(width);
	DEFINE_GETTER(width);
	DEFINE_SETTER(height);
	DEFINE_GETTER(height);
	DEFINE_SETTER(stride);
	DEFINE_GETTER(stride);
private:
	size_t width;
	size_t height;
	size_t stride;
};
	   	
struct RHI_TEXTURE_2D : public RHI_BUFFER {
	RHI_TEXTURE_2D(resource_state base_state, resource_format resource_format,
		size_t width, size_t height, size_t stride)
		: RHI_BUFFER(base_state, resource_format, width, height, stride) {}
};

struct RHI_CONSTANT_BUFFER : public RHI_BUFFER {

	RHI_CONSTANT_BUFFER(resource_state base_state, resource_format resource_format,
		size_t width, size_t height, size_t stride) : RHI_BUFFER(base_state, resource_format, width, height, stride) {
	}
};

struct RHI_DEPTH_BUFFER : public RHI_BUFFER {

	RHI_DEPTH_BUFFER(resource_state base_state, resource_format resource_format,
		size_t width, size_t height, size_t stride) : RHI_BUFFER(base_state, resource_format, width, height, stride) {}
};

struct RHI_VIEWPORT {
	float x;
	float y;
	float width;
	float height;
	float min_z;
	float max_z;
};

struct RHI_RENDER_PASS {

	RHI_STRUCT_BASE_INFO(RHI_RENDER_PASS, RHI_STRUCT_BASE_PARAMS(

		(RHI_DEVICE& device, std::shared_ptr<RHI_TEXTURE_2D> render_target)
		: device(device), render_target(render_target), depth_buffer(nullptr) {
	}
	))

	operator RHI_DEVICE& () { return device.get(); }
	operator RHI_TEXTURE_2D& () { return *render_target.get(); }
	operator RHI_DEPTH_BUFFER* () { return depth_buffer.get(); }
	operator RHI_GRAPHICS_PIPELINE* () { return pipeline.get(); }
	operator RHI_VIEWPORT& () { return view_port; }
	operator std::vector<RHI_CONSTANT_BUFFER*>& () { return constant_buffers; }
	void set_depth_buffer(RHI_DEPTH_BUFFER* db) { depth_buffer = std::observer_ptr<RHI_DEPTH_BUFFER>(db); }
	void set_pipeline(RHI_GRAPHICS_PIPELINE* pl) { pipeline = std::observer_ptr<RHI_GRAPHICS_PIPELINE>(pl); }
	void set_view_port(const RHI_VIEWPORT& vp) { view_port = vp; }
	void set_constant_buffers(std::vector<RHI_CONSTANT_BUFFER*>& buffers) { constant_buffers = buffers; }
private:
	std::reference_wrapper<RHI_DEVICE> device;
	std::shared_ptr<RHI_TEXTURE_2D> render_target;
	std::observer_ptr<RHI_DEPTH_BUFFER> depth_buffer;
	std::observer_ptr<RHI_GRAPHICS_PIPELINE> pipeline;
	std::vector<RHI_CONSTANT_BUFFER*> constant_buffers;
	RHI_VIEWPORT view_port;
};

struct RHI_WINDOW {
	RHI_WINDOW(RHI_VOID_PTR h, std::shared_ptr<RHI_WINDOW_CALLBACKS> window_callbacks)
		: handle(h)
		, callbacks(window_callbacks) {
	}
	operator RHI_WINDOW_CALLBACKS& () { return *callbacks.get(); }
	operator RHI_VOID_PTR() { return handle; }
private:
	std::shared_ptr<RHI_WINDOW_CALLBACKS> callbacks;
	RHI_VOID_PTR handle;
};

#endif