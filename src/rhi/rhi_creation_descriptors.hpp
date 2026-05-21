#ifndef __rhi_creation_descriptors_hpp__
#define __rhi_creation_descriptors_hpp__

#include "rhi_types.hpp"

struct RHI_PLATFORM_DESC
{
	RHI_VOID_PTR platform_desc_ptr = nullptr;
};

// Creation params
struct RHI_DEVICE_DESC : public RHI_PLATFORM_DESC
{
	int adapter_id = -1;
	unsigned long long features = device_features_none;
};

struct RHI_BUFFER_DESC : public RHI_PLATFORM_DESC
{

	RHI_BUFFER_DESC(RHI_DEVICE &dev)
		: device(dev)
		, length(0)
		, mips(1)
		, memory_type(buffer_memory_type_gpu_only)
		, default_state(resource_state_none)
		, format(resource_format_none)
		, type(buffer_type_raw)
	{
	}
	std::reference_wrapper<RHI_DEVICE> device;
	size_t length, mips, resource_slot;
	buffer_memory_type memory_type;
	resource_state default_state;
	resource_format format;
	buffer_type type;
};

struct RHI_BUFFER_2D_DESC : RHI_BUFFER_DESC {
	
	RHI_BUFFER_2D_DESC(RHI_DEVICE& device)
		: RHI_BUFFER_DESC(device)
		, width(width)
		, height(height) {}
	size_t width;
	size_t height;
};

struct RHI_DEPTH_BUFFER_DESC : RHI_BUFFER_2D_DESC
{

	RHI_DEPTH_BUFFER_DESC(RHI_DEVICE& device)
		: RHI_BUFFER_2D_DESC(device)
	{
	}
};

struct RHI_VERTEX_BUFFER_DESC : public RHI_BUFFER_DESC
{

	RHI_VERTEX_BUFFER_DESC(RHI_DEVICE &device)
		: RHI_BUFFER_DESC(device)
	{
	}
	size_t count;
	size_t stride;
};

struct RHI_INDEX_BUFFER_DESC : public RHI_BUFFER_DESC
{

	RHI_INDEX_BUFFER_DESC(RHI_DEVICE &device)
		: RHI_BUFFER_DESC(device)
	{
	}
	size_t count;
	size_t stride;
};

struct RHI_WINDOW_DESC : public RHI_PLATFORM_DESC
{

	char title[_MAX_FNAME];
	size_t width;
	size_t height;
	bool full_screen;
	std::shared_ptr<RHI_WINDOW_CALLBACKS> callbacks;
};

struct RHI_COMMAND_QUEUE_DESC : public RHI_PLATFORM_DESC
{
	RHI_COMMAND_QUEUE_DESC(RHI_DEVICE &device)
		: device(device)
	{
	}

	std::reference_wrapper<RHI_DEVICE> device;
};

struct RHI_SWAP_CHAIN_DESC : public RHI_PLATFORM_DESC
{

	RHI_SWAP_CHAIN_DESC(RHI_DEVICE &device, RHI_COMMAND_QUEUE &command_queue, RHI_WINDOW &wnd)
		: device(device), command_queue(command_queue), window(wnd), width(0), height(0), color_format(resource_format_none), allow_tearing(false), buffer_count(0)
	{
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

struct RHI_COMMAND_BUFFER_DESC : public RHI_PLATFORM_DESC
{

	RHI_COMMAND_BUFFER_DESC(RHI_DEVICE &device, RHI_COMMAND_QUEUE &command_queue)
		: device(device), command_queue(command_queue)
	{
	}

	std::reference_wrapper<RHI_DEVICE> device;
	std::reference_wrapper<RHI_COMMAND_QUEUE> command_queue;
};

struct RHI_INPUT_LAYOUT_DESC : public RHI_PLATFORM_DESC
{

	RHI_INPUT_LAYOUT_DESC(const std::string &name, resource_format format, unsigned int offset)
		: name(name), format(format), offset(offset) {}
	std::string name;
	resource_format format;
	unsigned int offset;
};

struct RHI_RASTER_PIPELINE_DESC : public RHI_PLATFORM_DESC
{

	RHI_RASTER_PIPELINE_DESC(RHI_DEVICE &dev, RHI_PIPELINE_LAYOUT &pipeline_layout, std::vector<RHI_INPUT_LAYOUT_DESC> &input_layouts, RHI_COMPILED_SHADER_BUFFER *vs = nullptr, RHI_COMPILED_SHADER_BUFFER *ps = nullptr, RHI_COMPILED_SHADER_BUFFER *gs = nullptr, RHI_COMPILED_SHADER_BUFFER *hs = nullptr, RHI_COMPILED_SHADER_BUFFER *ts = nullptr, RHI_COMPILED_SHADER_BUFFER *ds = nullptr)
		: device(dev), layout(pipeline_layout), layouts_desc(input_layouts), vertex_shader(vs), pixel_shader(ps), geometry_shader(gs), hull_shader(hs), tess_shader(ts), domain_shader(ds), topology(primitive_topology_none), surface_format(resource_format_R8G8B8A8_norm), depth_buffer_format(resource_format_d24_norm_s8_uint)
	{
	}

	std::reference_wrapper<RHI_DEVICE> device;
	std::reference_wrapper<RHI_PIPELINE_LAYOUT> layout;
	std::vector<RHI_INPUT_LAYOUT_DESC> layouts_desc;
	std::observer_ptr<RHI_COMPILED_SHADER_BUFFER> vertex_shader;
	std::observer_ptr<RHI_COMPILED_SHADER_BUFFER> pixel_shader;
	std::observer_ptr<RHI_COMPILED_SHADER_BUFFER> geometry_shader;
	std::observer_ptr<RHI_COMPILED_SHADER_BUFFER> hull_shader;
	std::observer_ptr<RHI_COMPILED_SHADER_BUFFER> tess_shader;
	std::observer_ptr<RHI_COMPILED_SHADER_BUFFER> domain_shader;
	primitive_topology topology;
	resource_format surface_format;
	resource_format depth_buffer_format;
};

struct RHI_MESH_SHADER_RASTER_PIPELINE_DESC : public RHI_PLATFORM_DESC
{

	RHI_MESH_SHADER_RASTER_PIPELINE_DESC(RHI_DEVICE &dev, RHI_COMPILED_SHADER_BUFFER *ms, RHI_COMPILED_SHADER_BUFFER *ass)
		: device(dev), mesh_shader(ms), as_shader(ass), topology(primitive_topology_none)
	{
	}

	std::reference_wrapper<RHI_DEVICE> device;
	std::observer_ptr<RHI_COMPILED_SHADER_BUFFER> as_shader;
	std::observer_ptr<RHI_COMPILED_SHADER_BUFFER> mesh_shader;
	std::observer_ptr<RHI_COMPILED_SHADER_BUFFER> pixel_shader;
	primitive_topology topology;
};

struct RHI_RT_SHADER_UNIT {

	std::string name_id;	// id
	std::unique_ptr<RHI_COMPILED_SHADER_BUFFER> blob;				// blob buffer
};

struct RHI_RT_HIT_GROUP_DESC {
	
	std::string group_id;
	RHI_RT_SHADER_UNIT closest_hit; // ClosestHit
	RHI_RT_SHADER_UNIT any_hit;	// AnyHit
	RHI_RT_SHADER_UNIT intersection; // Intersection
	
};


struct RHI_RT_PIPELINE_DESC : public RHI_PLATFORM_DESC
{

	RHI_RT_PIPELINE_DESC(RHI_DEVICE &dev, RHI_PIPELINE_LAYOUT& p_layout)
		: device(dev)
		, layout(p_layout) {
	}
	
	std::reference_wrapper<RHI_DEVICE> device;
	std::reference_wrapper<RHI_PIPELINE_LAYOUT> layout;
	std::vector<RHI_RT_HIT_GROUP_DESC> hit_groups;
	std::vector<RHI_RT_SHADER_UNIT> miss_shaders;
	RHI_RT_SHADER_UNIT ray_gen;
};

struct RHI_DESCRIPTOR_DESC : public RHI_PLATFORM_DESC
{

	resource_type resource_type;
	int pool_range_start;
	int pool_range_count;
};

struct RHI_PIPELINE_LAYOUT_DESC : public RHI_PLATFORM_DESC
{

	RHI_PIPELINE_LAYOUT_DESC(RHI_DEVICE &dev)
		: device(dev)
	{
	}

	std::reference_wrapper<RHI_DEVICE> device;
	std::vector<RHI_DESCRIPTOR_DESC> descriptors;
	raster_pipeline_shader_type shader_type;
};

struct RHI_TEXTURE_2D_DESC : public RHI_BUFFER_2D_DESC
{

	RHI_TEXTURE_2D_DESC(RHI_DEVICE &dev)
		: RHI_BUFFER_2D_DESC(dev) {}

	size_t dims, depth;
	bool is_cube_map;
};

struct RHI_FENCE_DESC : public RHI_PLATFORM_DESC
{

	RHI_FENCE_DESC(RHI_DEVICE &dev)
		: device(dev), flags(fence_flags_none), initial_value(0)
	{
	}
	std::reference_wrapper<RHI_DEVICE> device;
	fence_flags flags;
	int initial_value;
};

struct RHI_RT_BVH_DESC : public RHI_PLATFORM_DESC
{

	RHI_RT_BVH_DESC(RHI_DEVICE &dev,
					RHI_COMMAND_BUFFER &cmd_buffer, RHI_BUFFER &vertex_buffer,
					RHI_BUFFER *index_buffer)
		: device(dev), command_buffer(cmd_buffer), vertex_buffer(vertex_buffer), index_buffer(index_buffer) {}
	std::reference_wrapper<RHI_DEVICE> device;
	std::reference_wrapper<RHI_COMMAND_BUFFER> command_buffer;
	std::reference_wrapper<RHI_BUFFER> vertex_buffer;
	std::observer_ptr<RHI_BUFFER> index_buffer;
	
};

struct RT_GEOMETRY_INSTANCES_DESC : public RHI_PLATFORM_DESC
{

	RT_GEOMETRY_INSTANCES_DESC(RHI_DEVICE &dev,
							   RHI_COMMAND_BUFFER &cmd_buffer, RHI_RT_BVH &bvh)
		: device(dev), command_buffer(cmd_buffer), parent_bvh(bvh)
	{
	}
	std::reference_wrapper<RHI_DEVICE> device;
	std::reference_wrapper<RHI_COMMAND_BUFFER> command_buffer;
	std::reference_wrapper<RHI_RT_BVH> parent_bvh;
	std::vector<Eigen::Matrix4f> transforms;
	size_t resource_slot;
};

struct RHI_RENDER_PASS_DESC : public RHI_PLATFORM_DESC
{

	RHI_RENDER_PASS_DESC(RHI_DEVICE &device, std::shared_ptr<RHI_TEXTURE_2D> buffer)
		: device(device), render_target(buffer), format(resource_format_none), synchronized(false)
	{
	}
	std::shared_ptr<RHI_TEXTURE_2D> render_target;
	std::reference_wrapper<RHI_DEVICE> device;
	resource_format format;
	bool synchronized;
};


struct RHI_RT_SAMPLER_DESC {
	
	RHI_RT_SAMPLER_DESC(RHI_DEVICE& dev)
		: device(dev) {

	}
	std::reference_wrapper<RHI_DEVICE> device;
	size_t resource_slot;
};

struct RHI_RT_SBT_DESC {

	std::vector<std::string> ray_gen_ids;
	std::vector<std::string> miss_ids;
	std::vector<std::string> hit_group_ids;
};

#endif