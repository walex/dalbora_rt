#ifndef __rhi_types_hpp__
#define __rhi_types_hpp__

#include "rhi_defs.hpp"
#include <Eigen/Dense>

struct RHI_TEXTURE_2D;
struct RHI_DEVICE {
	RHI_STRUCT_BASE_INFO(RHI_DEVICE, () {})
};

struct RHI_SWAP_CHAIN {
	RHI_STRUCT_BASE_INFO(RHI_SWAP_CHAIN, () {})
	void add_render_target(std::shared_ptr<RHI_TEXTURE_2D> rt) {
		m_render_targets.push_back(rt);
	}
	std::shared_ptr<RHI_TEXTURE_2D> get_render_target(size_t index) {

		return m_render_targets.at(index);
	}
	size_t get_render_target_count() { return m_render_targets.size(); }
private:
	std::vector<std::shared_ptr<RHI_TEXTURE_2D>> m_render_targets;
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
	RHI_RT_PIPELINE(RHI_PIPELINE_LAYOUT& layout, std::unique_ptr<RHI_SHADER_TABLE_ENTIRES> sbt)
		: RHI_GRAPHICS_PIPELINE(pipeline_type_rt)
		, layout(layout)
		, shader_table(std::move(sbt)) {
	}
	operator RHI_PIPELINE_LAYOUT& () { return layout.get(); }
	operator RHI_SHADER_TABLE_ENTIRES& () { return  *shader_table; }
private:
	std::reference_wrapper<RHI_PIPELINE_LAYOUT> layout;
	std::unique_ptr<RHI_SHADER_TABLE_ENTIRES> shader_table;
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

		(resource_state default_state, resource_format resource_format)
		: current_state(default_state), default_state(default_state), format(resource_format) {
	}
	))
public:
	DEFINE_SETTER(current_state);
	DEFINE_GETTER(current_state);
	DEFINE_GETTER(default_state);
	DEFINE_GETTER(format);
private:
	resource_state current_state;
	resource_state default_state;
	resource_format format;
};

struct RHI_BUFFER: public RHI_RESOURCE {

	RHI_STRUCT_BASE_INFO(RHI_BUFFER, RHI_STRUCT_BASE_PARAMS(

		(resource_state default_state, resource_format resource_format, size_t length)
		: RHI_RESOURCE(default_state, resource_format)
		, length(length) {
	}
	))
public:
	DEFINE_SETTER(length);
	DEFINE_GETTER(length);
private:
	size_t length;
};
	   	
struct RHI_TEXTURE_MIPS {
	size_t width;
	size_t height;
	size_t offset;
	size_t num_rows;
	size_t pitch;
	size_t depth;
	resource_format format;
};

struct RHI_VERTEX_BUFFER : public RHI_BUFFER {

	RHI_VERTEX_BUFFER(resource_state base_state, resource_format resource_format,
		size_t length, size_t stride)
		: RHI_BUFFER(base_state, resource_format, length)
		, stride(stride) {
	}
	DEFINE_GETTER(stride);
private:
	size_t stride;
};

struct RHI_INDEX_BUFFER : public RHI_BUFFER {

	RHI_INDEX_BUFFER(resource_state base_state, resource_format resource_format,
		size_t length, size_t stride)
		: RHI_BUFFER(base_state, resource_format, length)
		, stride(stride) {
	}
	DEFINE_GETTER(stride);
private:
	size_t stride;
};

struct RHI_TEXTURE_2D : public RHI_BUFFER {

	RHI_TEXTURE_2D(resource_state base_state, resource_format resource_format,
		size_t width, size_t height, size_t phisycal_size,
		std::vector<RHI_TEXTURE_MIPS>&& mips)
		: RHI_BUFFER(base_state, resource_format, phisycal_size)
		, width(width)
		, height(height)
		, mipmaps(std::move(mips)) {
	}
	std::vector<RHI_TEXTURE_MIPS>& get_mips() { return mipmaps; };
	DEFINE_GETTER(width);
	DEFINE_GETTER(height);
private:
	size_t width, height;
	std::vector<RHI_TEXTURE_MIPS> mipmaps;
};

struct RHI_CONSTANT_BUFFER : public RHI_BUFFER {

	RHI_CONSTANT_BUFFER(resource_state base_state, resource_format resource_format,
		size_t length) : RHI_BUFFER(base_state, resource_format, length) {
	}
};

struct RHI_DEPTH_BUFFER : public RHI_BUFFER {

	RHI_DEPTH_BUFFER(resource_state base_state, resource_format resource_format,
		size_t width, size_t height) : RHI_BUFFER(base_state, resource_format, width * height) {}
	DEFINE_GETTER(width);
	DEFINE_GETTER(height);
private:
	size_t width, height;
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

		(RHI_DEVICE& device, std::shared_ptr<RHI_TEXTURE_2D> rt)
		: device(device), render_target(rt), depth_buffer(nullptr) {
	}
	))

	operator RHI_DEVICE& () { return device.get(); }
	operator RHI_TEXTURE_2D& () { return *render_target; }
	operator RHI_DEPTH_BUFFER* () { return depth_buffer.get(); }
	operator RHI_GRAPHICS_PIPELINE* () { return pipeline.get(); }
	operator RHI_VIEWPORT& () { return view_port; }
	operator std::vector<RHI_CONSTANT_BUFFER*>& () { return constant_buffers; }
	void set_depth_buffer(RHI_DEPTH_BUFFER* db) { depth_buffer = std::observer_ptr<RHI_DEPTH_BUFFER>(db); }
	void set_pipeline(RHI_GRAPHICS_PIPELINE* pl) { pipeline = std::observer_ptr<RHI_GRAPHICS_PIPELINE>(pl); }
	void set_view_port(const RHI_VIEWPORT& vp) { view_port = vp; }
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

struct RHI_RT_BVH {

	virtual ~RHI_RT_BVH() = default;
};

struct RHI_SAMPLER {
	virtual ~RHI_SAMPLER() = default;
};

struct RHI_RT_HIT_GROUP {
	virtual ~RHI_RT_HIT_GROUP() = default;
};

#endif