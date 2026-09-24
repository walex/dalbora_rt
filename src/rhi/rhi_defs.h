#ifndef __rhi_defs_h__
#define __rhi_defs_h__

enum rhi_api
{
	rhi_api_dx12
};

enum device_type
{
	device_type_none = 0,
	device_type_dx12,
	device_type_vk
};

enum resource_format
{
	resource_format_none = 0,
	resource_format_uint8,
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

enum shader_view_type
{
	shader_view_type_none,
	shader_view_type_bvh_buffer,
	shader_view_type_sampler,
	shader_view_type_render_target,
	shader_view_type_depth_stencil_target,
	shader_view_type_constant_buffer,
	shader_view_type_rw_buffer,
	shader_view_type_read_only_buffer,
	shader_view_type_rw_texture_buffer,
	shader_view_type_read_only_texture_buffer,
	shader_view_type_count
};

constexpr size_t resources_count = static_cast<size_t>(shader_view_type_count);

enum buffer_type
{

	buffer_type_undef = 0,
	buffer_type_raw,
	buffer_type_image_1d,
	buffer_type_image_2d,
	buffer_type_image_3d,
	buffer_type_bvh,
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
	queue_type_undef = 0,
	queue_type_graphics,
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

enum resource_flags {
	resource_flags_none = 0,
	resource_flags_shader_read_write = 0x1,
	resource_flags_render_target = 0x2
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

enum hlsl_shader_model
{
	hlsl_shader_model_6_0 = 0x60,
	hlsl_shader_model_6_1 = 0x61,
	hlsl_shader_model_6_2 = 0x62,
	hlsl_shader_model_6_4 = 0x64,
	hlsl_shader_model_6_5 = 0x65,
	hlsl_shader_model_6_6 = 0x66,
	hlsl_shader_model_6_7 = 0x67,
	hlsl_shader_model_6_8 = 0x68,
	hlsl_shader_model_6_9 = 0x69
};

enum memory_resource_type {
	memory_resource_type_descriptor_table,
	memory_resource_type_pool
};

enum memory_descriptor_type
{
	memory_descriptor_type_buffer, // VkDescriptorPool + VkDescriptor UNIFORM_BUFFER(CBV), SAMPLED_IMAGE / STORAGE_TEXEL_BUFFER(SRV) y STORAGE_IMAGE / STORAGE_BUFFER(UAV)
	memory_descriptor_type_sampler,
	memory_descriptor_type_rtv,
	memory_descriptor_type_dsv,
	memory_descriptor_type_count
};

enum shader_binding_signature
{
	shader_binding_signature_bindless_table = 0,
	shader_binding_signature_32bits_constants = 1,
	shader_binding_signature_constant_buffer = 2,
	shader_binding_signature_rw_buffer = 3,
	shader_binding_signature_read_only_buffer = 4,
	shader_binding_signature_count
};

constexpr __int64 device_features_none = 0x0;
constexpr __int64 device_features_raytracing = 0x1;
constexpr __int64 device_features_variable_rate_shading = 0x2;
constexpr __int64 device_features_mesh_shaders = 0x4;
constexpr __int64 device_features_enable_texture_sampling = 0x8;

#endif