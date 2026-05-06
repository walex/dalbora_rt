#ifndef __rhi_creation_descriptors_hpp__
#define __rhi_creation_descriptors_hpp__

#include "rhi_types.hpp"

// Creation params
struct RHI_DEVICE_DESC {
	int adapter_id = -1;
	unsigned long long features = device_features_none;
};

struct RHI_BUFFER_DESC {

	RHI_BUFFER_DESC(RHI_DEVICE& dev)
		: device(dev)
		, width(0)
		, height(0)
		, memory_type(buffer_memory_type_gpu_only)
		, initial_state(resource_state_none)
		, format(resource_format_none)
		, flags(buffer_resource_flags_none)
		, type(buffer_type_raw) {
	}
	std::reference_wrapper<RHI_DEVICE> device;
	size_t width, height;
	buffer_memory_type memory_type;
	resource_state initial_state;
	resource_format format;
	buffer_type type;
	buffer_resource_flags flags;
};

struct RHI_DEPTH_BUFFER_DESC : RHI_BUFFER_DESC {

	RHI_DEPTH_BUFFER_DESC(RHI_DEVICE& device, RHI_DESCRIPTOR_POOL* memory_pool = nullptr)
		: RHI_BUFFER_DESC(device)
		, pool(memory_pool)
		, slot(0)
	{
	}

	RHI_DESCRIPTOR_POOL* pool;
	int slot;
};

struct RHI_VERTEX_BUFFER_DESC : public RHI_BUFFER_DESC {

	RHI_VERTEX_BUFFER_DESC(RHI_DEVICE& device)
		: RHI_BUFFER_DESC(device) {
	}
	size_t stride;
	size_t count;
};

struct RHI_INDEX_BUFFER_DESC : public RHI_BUFFER_DESC {

	RHI_INDEX_BUFFER_DESC(RHI_DEVICE& device)
		: RHI_BUFFER_DESC(device) {
	}
	size_t stride;
	size_t count;
};

struct RHI_WINDOW_DESC {

	char title[_MAX_FNAME];
	size_t width;
	size_t height;
	bool full_screen;
	std::shared_ptr<RHI_WINDOW_CALLBACKS> callbacks;
};

struct RHI_COMMAND_QUEUE_DESC {
	RHI_COMMAND_QUEUE_DESC(RHI_DEVICE& device)
		: device(device) {
	}

	std::reference_wrapper<RHI_DEVICE> device;
};

struct RHI_SWAP_CHAIN_DESC {

	RHI_SWAP_CHAIN_DESC(RHI_DEVICE& device, RHI_COMMAND_QUEUE& command_queue, RHI_WINDOW& wnd)
		: device(device)
		, command_queue(command_queue)
		, window(wnd)
		, width(0)
		, height(0)
		, color_format(resource_format_none)
		, allow_tearing(false)
		, buffer_count(0) {
	}

	std::reference_wrapper<RHI_DEVICE> device;
	std::reference_wrapper<RHI_COMMAND_QUEUE> command_queue;
	std::reference_wrapper<RHI_WINDOW> window;
	size_t width;
	size_t height;
	resource_format color_format;
	bool allow_tearing;
	size_t buffer_count;
};

struct RHI_COMMAND_BUFFER_DESC {

	RHI_COMMAND_BUFFER_DESC(RHI_DEVICE& device, RHI_COMMAND_QUEUE& command_queue)
		: device(device)
		, command_queue(command_queue) {
	}

	std::reference_wrapper<RHI_DEVICE> device;
	std::reference_wrapper<RHI_COMMAND_QUEUE> command_queue;
};

struct RHI_INPUT_LAYOUT_DESC {

	std::string name;
	resource_format format;
	unsigned int offset;
};

struct RHI_RASTER_PIPELINE_DESC {

	RHI_RASTER_PIPELINE_DESC(RHI_DEVICE& dev
		, RHI_PIPELINE_LAYOUT& pipeline_layout
		, std::vector<RHI_INPUT_LAYOUT_DESC>& input_layouts
		, RHI_SHADER_BUFFER* vs = nullptr
		, RHI_SHADER_BUFFER* ps = nullptr
		, RHI_SHADER_BUFFER* gs = nullptr
		, RHI_SHADER_BUFFER* hs = nullptr
		, RHI_SHADER_BUFFER* ts = nullptr
		, RHI_SHADER_BUFFER* ds = nullptr)
		: device(dev)
		, layout(pipeline_layout)
		, layouts(input_layouts)
		, vertex_shader(vs)
		, pixel_shader(ps)
		, geometry_shader(gs)
		, hull_shader(hs)
		, tess_shader(ts)
		, domain_shader(ds)
		, topology(primitive_topology_none)
		, surface_format(resource_format_R8G8B8A8_norm) {
	}

	std::reference_wrapper<RHI_DEVICE> device;
	std::reference_wrapper<RHI_PIPELINE_LAYOUT> layout;
	std::vector<RHI_INPUT_LAYOUT_DESC> layouts;
	std::observer_ptr<RHI_SHADER_BUFFER> vertex_shader;
	std::observer_ptr<RHI_SHADER_BUFFER> pixel_shader;
	std::observer_ptr<RHI_SHADER_BUFFER> geometry_shader;
	std::observer_ptr<RHI_SHADER_BUFFER> hull_shader;
	std::observer_ptr<RHI_SHADER_BUFFER> tess_shader;
	std::observer_ptr<RHI_SHADER_BUFFER> domain_shader;
	primitive_topology topology;
	resource_format surface_format;
};

struct RHI_MESH_SHADER_RASTER_PIPELINE_DESC {

	RHI_MESH_SHADER_RASTER_PIPELINE_DESC(RHI_DEVICE& dev
		, RHI_SHADER_BUFFER* ms
		, RHI_SHADER_BUFFER* ass)
		: device(dev)
		, mesh_shader(ms)
		, as_shader(ass)
		, topology(primitive_topology_none) {
	}

	std::reference_wrapper<RHI_DEVICE> device;
	std::observer_ptr<RHI_SHADER_BUFFER> as_shader;
	std::observer_ptr<RHI_SHADER_BUFFER> mesh_shader;
	std::observer_ptr<RHI_SHADER_BUFFER> pixel_shader;
	primitive_topology topology;
};

struct RHI_RT_PIPELINE_DESC {

	RHI_RT_PIPELINE_DESC(RHI_DEVICE& dev)
		: device(dev) {
	}
	std::reference_wrapper<RHI_DEVICE> device;
};

struct RHI_DESCRIPTOR_POOL_DESC {

	RHI_DESCRIPTOR_POOL_DESC(RHI_DEVICE& dev)
		: device(dev)
		, slot_count(1)
		, shader_visibility(false) {
	}
	std::reference_wrapper<RHI_DEVICE> device;

	int slot_count;
	bool shader_visibility;
	resource_type resource_type;

};

struct RHI_DESCRIPTOR_DESC {

	resource_type resource_type;
	int pool_range_start;
	int pool_range_count;
};

struct RHI_PIPELINE_LAYOUT_DESC {

	RHI_PIPELINE_LAYOUT_DESC(RHI_DEVICE& dev)
		: device(dev) {
	}

	std::reference_wrapper<RHI_DEVICE> device;
	std::vector<RHI_DESCRIPTOR_DESC> descriptors;
	raster_pipeline_shader_type shader_type;
};

struct RHI_TEXTURE_2D_DESC {

	RHI_TEXTURE_2D_DESC(RHI_DEVICE& dev)
		: device(dev)
		, width(0)
		, height(0)
		, format(resource_format_none) {
	}
	std::reference_wrapper<RHI_DEVICE> device;
	size_t width;
	size_t height;
	resource_format format;
};

struct RHI_FENCE_DESC {

	RHI_FENCE_DESC(RHI_DEVICE& dev)
		: device(dev)
		, flags(fence_flags_none)
		, initial_value(0) {
	}
	std::reference_wrapper<RHI_DEVICE> device;
	fence_flags flags;
	int initial_value;
};

struct RHI_RT_BVH_DESC {

	RHI_RT_BVH_DESC(RHI_DEVICE& dev, RHI_COMMAND_QUEUE& cmd_queue,
		RHI_COMMAND_BUFFER& cmd_buffer, RHI_GEOMETRY& geo_buffer)
		: device(dev)
		, command_queue(cmd_queue)
		, command_buffer(cmd_buffer)
		, geometry_buffer(geo_buffer) {
	}
	std::reference_wrapper<RHI_DEVICE> device;
	std::reference_wrapper<RHI_COMMAND_QUEUE> command_queue;
	std::reference_wrapper<RHI_COMMAND_BUFFER> command_buffer;
	std::reference_wrapper<RHI_GEOMETRY> geometry_buffer;
};

struct RHI_RENDER_PASS_DESC {

	RHI_RENDER_PASS_DESC(RHI_DEVICE& dev, RHI_DESCRIPTOR_POOL& memory_pool
		, std::shared_ptr<RHI_TEXTURE_2D> buffer)
		: device(dev)
		, pool(memory_pool)
		, render_target(buffer)
		, format(resource_format_none)
		, buffer_index(-1)
		, synchronized(false) {
	}
	std::shared_ptr<RHI_TEXTURE_2D> render_target;
	std::reference_wrapper<RHI_DEVICE> device;
	std::reference_wrapper<RHI_DESCRIPTOR_POOL> pool;
	resource_format format;
	int buffer_index;
	bool synchronized;
};

#endif