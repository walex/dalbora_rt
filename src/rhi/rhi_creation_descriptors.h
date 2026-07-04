#ifndef __rhi_creation_descriptors_h__
#define __rhi_creation_descriptors_h__

#include "rhi_types.h"

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
	resource_flags flags = resource_flags_none;

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
	char* title;
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
	size_t width = 0;
	size_t height = 0;
	resource_format color_format = resource_format_none;
	bool disable_vsync = false;
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
	primitive_topology topology = primitive_topology_none;
	resource_format format = resource_format_none;
	resource_format depth_buffer_format = resource_format_none;
};

struct RHI_MESH_SHADER_RASTER_PIPELINE_DESC  {
	RHI_DEVICE* device = nullptr;
	RHI_COMPILED_SHADER_BUFFER* as_shader = nullptr;
	RHI_COMPILED_SHADER_BUFFER* mesh_shader = nullptr;
	RHI_COMPILED_SHADER_BUFFER* pixel_shader = nullptr;
	primitive_topology topology = primitive_topology_none;
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
	char name_id[MAX_SHADER_UNIT_NAME_LENGTH];
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

struct RHI_SHADER_DESCRIPTOR_DESC  {
	resource_type resource_type = resource_type_generic_rw_buffer;
	size_t shader_register_start = 0;
	size_t shader_register_max = 0;
};

#define MAX_PIPELINE_DESCRIPTORS 16
struct RHI_PIPELINE_LAYOUT_DESC  {
	RHI_DEVICE* device = nullptr;
	primitive_topology topology = primitive_topology_none;
	resource_format	surface_format = resource_format_none;
	resource_format depth_buffer_format = resource_format_none;
	RHI_SHADER_DESCRIPTOR_DESC descriptors[MAX_PIPELINE_DESCRIPTORS];
	size_t descriptor_count = 0;
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
	RHI_BUFFER*const* vertex_buffer = nullptr;
	RHI_BUFFER*const* index_buffer = nullptr;
	size_t indices_length = 0;
	size_t count = 0;
};

struct RHI_RT_BVH_GEOMETRY_INSTANCE_DESC {
	RHI_RT_BVH* parent_bvh = nullptr;;
	size_t transforms_count = 0;;
	const float*const* transforms = nullptr;
	size_t update_index = 0;
};

struct RHI_RT_BVH_GEOMETRY_DESC  {
	RHI_DEVICE* device = nullptr;
	RHI_COMMAND_BUFFER* command_buffer = nullptr;
	RHI_RT_BVH_GEOMETRY_INSTANCE_DESC* instance_info;
	size_t instance_info_count;
	size_t total_instances;
	bool read_only;
};

struct RHI_RENDER_PASS_DESC  {
	RHI_DEVICE* device = nullptr;
};

struct RHI_RT_SAMPLER_DESC {
	RHI_DEVICE* device = nullptr;
};

#define MAX_SBT_RAY_GEN_ENTRIES 8
#define MAX_SBT_MISS_ENTRIES 8
#define MAX_SBT_GROUPS_ENTRIES 16
struct RHI_RT_SBT_DESC {
	const char* ray_gen_ids[MAX_SBT_RAY_GEN_ENTRIES];
	size_t ray_gen_count = 0;
	const char* miss_ids[MAX_SBT_MISS_ENTRIES];
	size_t miss_shader_count = 0;
	const char* hit_group_ids[MAX_SBT_GROUPS_ENTRIES];
	size_t hit_group_count = 0;
};

struct RHI_VIEW_DESC  {
	resource_type type = resource_type_generic_rw_buffer;
	resource_format format = resource_format_none;
	size_t mip_maps_count  = 0;
	RHI_DEVICE* device = nullptr;
	RHI_BUFFER* buffer = nullptr;
	size_t slot_id = 0;
};
#endif