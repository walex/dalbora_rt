#ifndef __rhi_types_hpp__
#define __rhi_types_hpp__

#include "rhi_defs.hpp"

struct RHI_HANDLE {
	virtual ~RHI_HANDLE() = default;
	virtual void set_handle(RHI_VOID_PTR handle) = 0;
	template<typename T>
	operator T* () const { 
		return static_cast<T*>(this->get_handle());
	}
	virtual RHI_VOID_PTR get_handle() const = 0;
};

struct RHI_BUFFER { 
	virtual ~RHI_BUFFER() = default;
	size_t length = 0;
	size_t stride = 0;
	resource_format format = resource_format_none;
};

struct RHI_VIEW {
	virtual ~RHI_VIEW() = default;
	RHI_BUFFER* resource = nullptr;
};

struct RHI_DEVICE {
	virtual ~RHI_DEVICE() = default;
};

struct RHI_TEXTURE_MIPS {
	size_t width = 0;
	size_t height = 0;
	size_t offset = 0;
	size_t num_rows = 0;
	size_t pitch = 0;
	size_t depth = 0;
	resource_format format = resource_format_none;
};

#define MAX_TEXTURE_MIP_LEVELS 16
struct RHI_TEXTURE_2D {
	virtual ~RHI_TEXTURE_2D() = default;
	size_t width = 0;
	size_t height = 0;
	RHI_TEXTURE_MIPS mip_maps[MAX_TEXTURE_MIP_LEVELS];
	size_t mip_maps_count = 0;
};

#define MAX_RENDER_TARGETS 8
struct RHI_SWAP_CHAIN {
	virtual ~RHI_SWAP_CHAIN() = default;
	std::unique_ptr<RHI_VIEW> render_targets[MAX_RENDER_TARGETS];
	size_t render_targets_count = 0;
};

struct RHI_COMPILED_SHADER_BUFFER {
	virtual ~RHI_COMPILED_SHADER_BUFFER() = default;
};

struct RHI_PIPELINE_LAYOUT {
	virtual ~RHI_PIPELINE_LAYOUT() = default;
};

struct RHI_COMMAND_BUFFER {
	virtual ~RHI_COMMAND_BUFFER() = default;
};

struct RHI_FENCE {
	virtual ~RHI_FENCE() = default;
};

struct RHI_PIPELINE { 
	virtual ~RHI_PIPELINE() = default;
	RHI_PIPELINE_LAYOUT* layout = nullptr; 
};

struct RHI_RASTER_PIPELINE : public RHI_PIPELINE {
	virtual ~RHI_RASTER_PIPELINE() = default;
};

#define MAX_SHADER_TABLE_ENTRIES 16
#define MAX_SHADER_ENTRY_NAME_LENGTH 64
struct RHI_SHADER_TABLE_ENTRY {
	virtual ~RHI_SHADER_TABLE_ENTRY() = default;
	char name[MAX_SHADER_ENTRY_NAME_LENGTH];
	RHI_VOID_PTR shader_id = nullptr;
};

struct RHI_RT_PIPELINE : RHI_PIPELINE {
	virtual ~RHI_RT_PIPELINE() = default;
	RHI_SHADER_TABLE_ENTRY shader_table[MAX_SHADER_TABLE_ENTRIES];
	size_t shader_table_entries_count = 0;
};

struct RHI_COMMAND_QUEUE {
	virtual ~RHI_COMMAND_QUEUE() = default;
	std::unique_ptr<RHI_FENCE> fence;
	uint64_t fence_counter = 0;
};

struct RHI_VIEWPORT {
	virtual ~RHI_VIEWPORT() = default;
	float x = 0;
	float y = 0;
	float width = 0;
	float height = 0;
	float min_z = 0;
	float max_z = 0;
};

struct RHI_RENDER_PASS {
	virtual ~RHI_RENDER_PASS() = default;
	RHI_DEVICE* device = nullptr;
	RHI_VIEW* render_target_view = nullptr;
	RHI_VIEW* depth_buffer_view = nullptr;
	RHI_PIPELINE* pipeline = nullptr;
	RHI_VIEWPORT view_port = {};
};

struct RHI_WINDOW {
	virtual ~RHI_WINDOW() = default;
	RHI_VOID_PTR handle = nullptr;
	RHI_WINDOW_CALLBACKS* callbacks = nullptr;
};

struct RHI_RT_BVH {
	virtual ~RHI_RT_BVH() = default;
};

struct RHI_SAMPLER {
	virtual ~RHI_SAMPLER() = default;
};

struct RHI_SBT_TABLE
{
	size_t ray_gen_offset = 0, miss_offset = 0, hit_group_offset = 0;
	size_t ray_gen_size = 0, miss_size = 0, hit_group_size = 0;
	size_t record_size = 0;
};
#endif