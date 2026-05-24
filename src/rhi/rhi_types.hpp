#ifndef __rhi_types_hpp__
#define __rhi_types_hpp__

#include "rhi_defs.hpp"

struct RHI_HANDLE {
	virtual void set_handle(RHI_VOID_PTR handle) = 0;
	template<typename T>
	operator T* () const { return static_cast<T*>(this->get_handle()); }
	virtual RHI_VOID_PTR get_handle() const = 0;
};

struct RHI_DEVICE {};
struct RHI_BUFFER  { size_t length; resource_format format;};
struct RHI_TEXTURE_MIPS {
	size_t width;
	size_t height;
	size_t offset;
	size_t num_rows;
	size_t pitch;
	size_t depth;
	resource_format format;
};
#define MAX_TEXTURE_MIP_LEVELS 16
struct RHI_TEXTURE_2D : public RHI_BUFFER {
	size_t width, height;
	RHI_TEXTURE_MIPS mip_maps[MAX_TEXTURE_MIP_LEVELS];
	size_t mip_maps_count = 0;
};
#define MAX_RENDER_TARGETS 8
struct RHI_SWAP_CHAIN {
	std::unique_ptr<RHI_VIEW> render_targets[MAX_RENDER_TARGETS];
	size_t render_targets_count = 0;
};
struct RHI_COMPILED_SHADER_BUFFER {};
struct RHI_PIPELINE_LAYOUT {};
struct RHI_COMMAND_BUFFER {};
struct RHI_FENCE {};
struct RHI_PIPELINE { RHI_PIPELINE_LAYOUT* layout; };
struct RHI_RASTER_PIPELINE : public RHI_PIPELINE {};
#define MAX_SHADER_TABLE_ENTRIES 16
#define MAX_SHADER_ENTRY_NAME_LENGTH 64
struct RHI_SHADER_TABLE_ENTRY {
	char name[MAX_SHADER_ENTRY_NAME_LENGTH];
	RHI_VOID_PTR shader_id;
};
struct RHI_RT_PIPELINE : RHI_PIPELINE {
	RHI_SHADER_TABLE_ENTRY shader_table[MAX_SHADER_TABLE_ENTRIES];
	size_t shader_table_entries_count = 0;
};
struct RHI_COMMAND_QUEUE {
	std::unique_ptr<RHI_FENCE> fence;
	uint64_t fence_counter = 0;
};

struct RHI_VERTEX_BUFFER : public RHI_BUFFER { size_t stride; };
struct RHI_INDEX_BUFFER : public RHI_BUFFER { size_t stride; };

typedef RHI_BUFFER RHI_CONSTANT_BUFFER;
struct RHI_DEPTH_BUFFER : public RHI_BUFFER {
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
	RHI_DEVICE* device;
	RHI_VIEW* render_target_view;
	RHI_VIEW* depth_buffer_view;
	RHI_PIPELINE* pipeline;
	RHI_VIEWPORT view_port;
};
struct RHI_WINDOW {
	RHI_VOID_PTR handle;
	RHI_WINDOW_CALLBACKS* callbacks;
};
struct RHI_RT_BVH {};
struct RHI_SAMPLER {};
struct RHI_VIEW {
	std::shared_ptr<RHI_BUFFER> resource;
};
struct RHI_SBT_TABLE
{
	size_t ray_gen_offset, miss_offset, hit_group_offset;
	size_t ray_gen_size, miss_size, hit_group_size;
	size_t record_size;
};
#endif