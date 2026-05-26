#ifndef __rhi_creation_descriptors_hpp__
#define __rhi_creation_descriptors_hpp__

#include "rhi_types.hpp"

// Creation params
struct RHI_DEVICE_DESC  {
	int adapter_id = -1;
	unsigned long long features = device_features_none;
};

struct RHI_BUFFER_DESC  {
	RHI_DEVICE* device = nullptr;
	size_t length = 0, mips = 0, stride = 0;
	buffer_memory_type memory_type = buffer_memory_type_default;
	resource_format format = resource_format_none;
	buffer_type type = buffer_type_undef;

};

struct RHI_BUFFER_2D_DESC : RHI_BUFFER_DESC {
	size_t width = 0;
	size_t height = 0;
	bool is_render_target = false;
};

struct RHI_VERTEX_BUFFER_DESC : public RHI_BUFFER_DESC {
	size_t count;
};

struct RHI_INDEX_BUFFER_DESC : public RHI_BUFFER_DESC {
	size_t count;
};

struct RHI_WINDOW_DESC  {
	char title[_MAX_FNAME];
	size_t width;
	size_t height;
	bool full_screen;
	RHI_WINDOW_CALLBACKS* callbacks;
};

struct RHI_COMMAND_QUEUE_DESC  {
	RHI_DEVICE* device = nullptr;
};

struct RHI_SWAP_CHAIN_DESC  {
	RHI_DEVICE* device = nullptr;
	RHI_COMMAND_QUEUE* command_queue = nullptr;
	RHI_WINDOW* window = nullptr;
	size_t width = -1;
	size_t height = -1;
	resource_format color_format = resource_format_none;
	bool allow_tearing = false;
	size_t buffer_count = 0;
};

struct RHI_COMMAND_BUFFER_DESC  {
	RHI_DEVICE* device = nullptr;
	RHI_COMMAND_QUEUE* command_queue = nullptr;
};

#define MAX_INPUT_NAME_LENGTH 64
struct RHI_INPUT_LAYOUT_DESC  {
	char name[MAX_INPUT_NAME_LENGTH];
	resource_format format = resource_format_none;
	size_t offset = 0;
};

#define MAX_INPUT_LAYOUT_DESC 64
struct RHI_RASTER_PIPELINE_DESC  {
	RHI_DEVICE* device = nullptr;
	RHI_PIPELINE_LAYOUT* layout = nullptr;
	RHI_INPUT_LAYOUT_DESC input_layouts_desc[MAX_INPUT_LAYOUT_DESC];
	size_t input_layouts_desc_count = 0;
	RHI_COMPILED_SHADER_BUFFER* vertex_shader = nullptr;
	RHI_COMPILED_SHADER_BUFFER* pixel_shader = nullptr;
	RHI_COMPILED_SHADER_BUFFER* geometry_shader = nullptr;
	RHI_COMPILED_SHADER_BUFFER* hull_shader = nullptr;
	RHI_COMPILED_SHADER_BUFFER* tess_shader = nullptr;
	RHI_COMPILED_SHADER_BUFFER* domain_shader = nullptr;
	primitive_topology topology;
	resource_format surface_format;
	resource_format depth_buffer_format;
};

struct RHI_MESH_SHADER_RASTER_PIPELINE_DESC  {
	RHI_DEVICE* device = nullptr;
	RHI_COMPILED_SHADER_BUFFER* as_shader = nullptr;
	RHI_COMPILED_SHADER_BUFFER* mesh_shader = nullptr;
	RHI_COMPILED_SHADER_BUFFER* pixel_shader = nullptr;
	primitive_topology topology;
};

#define MAX_SHADER_UNIT_NAME_LENGTH 64
#define MAX_HIT_GROUPS 16
#define MAX_MISS_GROUPS 16
#define MAX_RAY_GEN_GROUPS 1
struct RHI_RT_SHADER_UNIT_DESC {
	char name_id[MAX_SHADER_UNIT_NAME_LENGTH];
	RHI_COMPILED_SHADER_BUFFER* blob = nullptr;
};

struct RHI_RT_HIT_GROUP_DESC {
	char group_id[MAX_SHADER_UNIT_NAME_LENGTH];
	RHI_RT_SHADER_UNIT_DESC closest_hit;
	RHI_RT_SHADER_UNIT_DESC any_hit;
	RHI_RT_SHADER_UNIT_DESC intersection;	
};

struct RHI_RT_PIPELINE_DESC  {
	RHI_DEVICE* device = nullptr;;
	RHI_PIPELINE_LAYOUT* layout = nullptr;;
	RHI_RT_HIT_GROUP_DESC hit_groups[MAX_HIT_GROUPS];
	size_t hit_group_count = 0;
	RHI_RT_SHADER_UNIT_DESC miss_shaders[MAX_MISS_GROUPS];
	size_t miss_shader_count = 0;
	RHI_RT_SHADER_UNIT_DESC ray_gen[MAX_RAY_GEN_GROUPS];
	size_t ray_gen_count = 0;
};

struct RHI_DESCRIPTOR_DESC  {
	resource_type resource_type;
	int pool_range_start = -1;
	int pool_range_count = -1;
};

#define MAX_PIPELINE_DESCRIPTORS 16
struct RHI_PIPELINE_LAYOUT_DESC  {
	RHI_DEVICE* device = nullptr;
	RHI_DESCRIPTOR_DESC descriptors[MAX_PIPELINE_DESCRIPTORS];
	size_t descriptor_count = 0;
	raster_pipeline_shader_type shader_type = shader_type_undef;
};

struct RHI_TEXTURE_2D_DESC : public RHI_BUFFER_2D_DESC {
	size_t dims, depth;
	bool is_cube_map;
};

struct RHI_FENCE_DESC  {
	RHI_DEVICE* device = nullptr;
	fence_flags flags;
	int initial_value;
};

struct RHI_RT_BVH_DESC  {
	RHI_DEVICE* device = nullptr;
	RHI_COMMAND_BUFFER* command_buffer = nullptr;
	RHI_BUFFER* vertex_buffer = nullptr;
	RHI_BUFFER* index_buffer = nullptr;
};

struct RT_GEOMETRY_INSTANCES_DESC  {
	RHI_DEVICE* device = nullptr;
	RHI_COMMAND_BUFFER* command_buffer = nullptr;
	RHI_RT_BVH* parent_bvh = nullptr;
	Eigen::Matrix4f* transforms;
	size_t instance_count = 0;
};

struct RHI_RENDER_PASS_DESC  {
	RHI_VIEW* render_target_view = nullptr;
	RHI_DEVICE* device = nullptr;
};

struct RHI_RT_SAMPLER_DESC {
	RHI_DEVICE* device = nullptr;
};

struct RHI_RT_SBT_DESC {
	char** ray_gen_ids = nullptr;
	size_t ray_gen_count = 0;
	char** miss_ids = nullptr;
	size_t miss_shader_count = 0;
	char** hit_group_ids = nullptr;
	size_t hit_group_count = 0;
};

struct RHI_VIEW_DESC  {
	resource_type type;
	resource_format format;
	RHI_DEVICE* device;
	RHI_BUFFER* buffer;
};
#endif