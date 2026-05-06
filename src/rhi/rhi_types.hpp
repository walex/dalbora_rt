#ifndef __rhi_types_hpp__
#define __rhi_types_hpp__

#include "rhi_defs.hpp"
#include <Eigen/Dense>

RHI_STRUCT(RHI_DEVICE, () {})
RHI_STRUCT(RHI_SWAP_CHAIN, () {})
RHI_STRUCT(RHI_RASTER_PIPELINE, () {})
RHI_STRUCT(RHI_RT_PIPELINE, () {})
RHI_STRUCT(RHI_FENCE, () {})
RHI_STRUCT(RHI_SHADER_BUFFER, () {})
RHI_STRUCT(RHI_PIPELINE_LAYOUT, () {})
RHI_STRUCT(RHI_COMMAND_BUFFER, () {})

struct RHI_COMMAND_QUEUE {
	RHI_STRUCT_BASE_INFO(RHI_COMMAND_QUEUE, () {})
public:
	RHI_COMMAND_QUEUE(std::unique_ptr<RHI_FENCE>&& fence)
		: m_fence(std::move(fence))
		, m_fence_counter(0) {
	}
	RHI_FENCE& get_fence() { return *m_fence.get(); }
	unsigned __int64 increment_fence_counter() { return ++m_fence_counter; }
	unsigned __int64 get_fence_counter() { return m_fence_counter; }
private:
	std::unique_ptr<RHI_FENCE> m_fence;
	unsigned __int64 m_fence_counter;
};

struct RHI_RESOURCE {
	RHI_STRUCT_BASE_INFO(RHI_RESOURCE, RHI_STRUCT_BASE_PARAMS(

		() : current_state(resource_state_none), format(resource_format_none) {
	}
	))
public:
	DEFINE_SETTER(current_state);
	DEFINE_GETTER(current_state);
	DEFINE_SETTER(format);
	DEFINE_GETTER(format);
private:
	resource_state current_state;
	resource_format format;
};

struct RHI_BUFFER: public RHI_RESOURCE {

	RHI_STRUCT_BASE_INFO(RHI_BUFFER, RHI_STRUCT_BASE_PARAMS(

		(size_t length, size_t stride) : length(length), stride(stride) {
	}
	))
public:
	DEFINE_SETTER(length);
	DEFINE_GETTER(length);
	DEFINE_SETTER(stride);
	DEFINE_GETTER(stride);
private:
	size_t length;
	size_t stride;
};
	   	
struct RHI_TEXTURE_2D : public RHI_BUFFER {

	RHI_TEXTURE_2D(size_t width, size_t height) : RHI_BUFFER(width*height, 0) {}
};

struct RHI_DEPTH_BUFFER : public RHI_BUFFER {

	RHI_DEPTH_BUFFER(size_t length, size_t stride) : RHI_BUFFER(length, stride) {}
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
	void set_depth_buffer(RHI_DEPTH_BUFFER* db) { depth_buffer = std::observer_ptr<RHI_DEPTH_BUFFER>(db); }
private:
	std::reference_wrapper<RHI_DEVICE> device;
	std::shared_ptr<RHI_TEXTURE_2D> render_target;
	std::observer_ptr<RHI_DEPTH_BUFFER> depth_buffer;
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