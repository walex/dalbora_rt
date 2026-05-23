#ifndef __rhi_defs_hpp__
#define __rhi_defs_hpp__

#include <functional>
#include <map>
#include <string>
struct RHI_WINDOW;
struct RHI_COMMAND_BUFFER;
typedef void* RHI_VOID_PTR;
using fptr_window_main_loop_callback = std::function<void(const RHI_WINDOW &window)>;
using fptr_window_on_init = std::function<void(const RHI_WINDOW &window)>;
using fptr_window_on_end = std::function<void(const RHI_WINDOW &window)>;
using fptr_command_queue_on_execute = std::function<void(RHI_VOID_PTR native_command_queue_impl,
														 std::vector<RHI_COMMAND_BUFFER *> &command_buffer_list)>;
using fptr_command_buffer_on_record = std::function<void(RHI_VOID_PTR native_command_buffer_impl)>;
using fptr_render_pass_on_execute = std::function<void()>;

enum rhi_api
{
	rhi_api_dx12
};

enum device_type
{
	device_type_none = 0,
	device_type_dx12
};

enum resource_format
{
	resource_format_none = 0,
	resource_format_uint16,
	resource_format_uint32,
	resource_format_R8G8B8A8_norm,
	resource_format_float,
	resource_format_float2,
	resource_format_float3,
	resource_format_float4,
	resource_format_d32_float_s8_uint,
	resource_format_d24_norm_s8_uint,
	resource_format_32_float,
	resource_format_d16_norm,
	resource_format_bc1_norm
};

enum resource_type
{
	resource_type_generic_rw_buffer = 1,
	resource_type_rt_bvh_buffeer = 2,
	resource_type_sampler = 3,
	resource_type_render_target = 4,
	resource_type_depth_stencil_target = 5,
	resource_type_constant_buffer = 6,
	resource_type_shader = 7,
};

enum buffer_type
{

	buffer_type_undef = 0,
	buffer_type_raw,
	buffer_type_image_1d,
	buffer_type_image_2d,
	buffer_type_image_3d,
	buffer_type_rt_bvh,
	buffer_type_depth_stencil
};

enum buffer_memory_type
{
	buffer_memory_type_default = 0,
	buffer_memory_type_gpu_only = 0,
	buffer_memory_type_shared_rw,
	buffer_memory_type_shared_read_only
};

enum queue_type
{
	queue_type_graphics = 0,
	queue_type_compute,
	queue_type_copy
};

enum primitive_topology
{
	primitive_topology_none = 0,
	primitive_topology_point,
	primitive_topology_line,
	primitive_topology_triangle,
	primitive_topology_patch
};

enum resource_state
{
	resource_state_none = 0,
	resource_state_raster_render_target,
	resource_state_depth_read,
	resource_state_depth_write,
	resource_state_shader_write,
	resource_state_shader_read,
	resource_state_copy_src,
	resource_state_copy_dest,
	resource_state_present,
	resource_state_rt_bvh,
	resource_state_constant_buffer,
	resource_state_vertex_buffer,
	resource_state_index_buffer,
	resource_state_generic_read,
	resource_state_rt_render_target,
};

enum fence_flags
{
	fence_flags_none = 0,
	fence_flags_shared = 1
};

enum raster_pipeline_shader_type
{

	shader_type_undef = 0,
	shader_type_vertex = 1,
	shader_type_hull = 2,
	shader_type_domain = 3,
	shader_type_geometry = 4,
	shader_type_pixel = 5,
	shader_type_amplification = 6,
	shader_type_mesh = 7
};

constexpr __int64 device_features_none = 0x0;
constexpr __int64 device_features_raytracing = 0x1;
constexpr __int64 device_features_variable_rate_shading = 0x2;
constexpr __int64 device_features_mesh_shaders = 0x4;

struct RHI_WINDOW_CALLBACKS
{

	fptr_window_on_init on_init;
	fptr_window_main_loop_callback main_loop;
	fptr_window_on_end on_end;
};

#endif