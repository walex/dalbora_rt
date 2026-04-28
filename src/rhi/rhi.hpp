#ifndef __rhi_hpp__
#define __rhi_hpp__

#include <memory>
#include <vector>
#include <Eigen/Dense>

enum device_type {
	device_type_none = 0,
	device_type_dx12
};

enum resource_format {
	resource_format_none = 0,
	resource_format_uint16,
	resource_format_uint32,
	resource_format_R8G8B8A8,
	resource_format_R8G8B8A8_float
};

enum buffer_memory_type {
	buffer_memory_type_gpu_only = 0,
	buffer_memory_type_cpu_to_gpu,
	buffer_memory_type_gpu_to_cpu
};

enum queue_type {
	queue_type_graphics = 0,
	queue_type_compute,
	queue_type_copy
};

enum vertex_format {
	vertex_format_None = 0,
	vertex_format_Position,
	vertex_format_Color,
	vertex_format_Texture,
	vertex_format_Normal
};

enum primitive_topology {
	primitive_topology_none = 0,
	primitive_topology_point,
	primitive_topology_line,
	primitive_topology_triangle,
	primitive_topology_patch
};

enum resource_state {
	resource_state_none = 0,
	resource_state_render_target,
	resource_state_depth_read,
	resource_state_depth_write,
	resource_state_shader_write,
	resource_state_shader_read,
	resource_state_copy_src,
	resource_state_copy_dest,
	resource_state_present,
	resource_state_rt_bvh,
	resource_state_constant_buffer,
	resource_state_generic_read
};

enum fence_flags {
	fence_flags_none = 0,
	fence_flags_shared = 1
};

constexpr __int64 device_features_none = 0x0;
constexpr __int64 device_features_raytracing = 0x1;
constexpr __int64 device_features_variable_rate_shading = 0x2;
constexpr __int64 device_features_mesh_shaders = 0x4;

struct RHI_NATIVE_HANDLE {
	virtual ~RHI_NATIVE_HANDLE() = default;
	virtual void* get_native_handle() = 0;
};

struct RHI_OBJECT {
	virtual ~RHI_OBJECT() = default;
	RHI_OBJECT(RHI_NATIVE_HANDLE* ptr) {
		native_impl.reset(ptr);
	}
	std::unique_ptr<RHI_NATIVE_HANDLE>& get_native_impl() {
		return native_impl;
	}
	
private:
	std::unique_ptr<RHI_NATIVE_HANDLE> native_impl;
};

struct RHI_RESOURCE : public RHI_OBJECT {
	RHI_RESOURCE(RHI_NATIVE_HANDLE* ptr)
		: RHI_OBJECT(ptr)
		, device(nullptr) {}
	resource_state get_current_state() const {
		return current_state;
	}
	void set_device(RHI_OBJECT* dev) {
		device = dev;
	}
	RHI_OBJECT* get_device() {
		return device;
	}
protected:
	void set_current_state(resource_state state) {
		current_state = state;
	}	
private:
	resource_state current_state = resource_state_none;
	resource_format format = resource_format_none;
	RHI_OBJECT* device;
};

struct RHI_BUFFER_RESOURCE : public RHI_RESOURCE {
	RHI_BUFFER_RESOURCE(RHI_NATIVE_HANDLE* ptr)
		: RHI_RESOURCE(ptr)
		, size(0)
		, stride(0) {
	}
	size_t size;
	size_t stride;
};

struct RHI_GEOMETRY {
	RHI_GEOMETRY(RHI_RESOURCE& vb, RHI_RESOURCE* ib = nullptr)
		: vbo(&vb)
		, ibo(ib) {}
	RHI_RESOURCE* get_vertex_buffer() {
		return vbo;
	}
	RHI_RESOURCE* get_index_buffer() {
		return ibo;
	}
	void set_transforms(std::vector<Eigen::Matrix4f>&& t) {
		transforms = std::move(t);
	}
	const std::vector<Eigen::Matrix4f>& get_transforms() {
		return transforms;
	}
private:
	RHI_RESOURCE* vbo;
	RHI_RESOURCE* ibo;
	std::vector<Eigen::Matrix4f> transforms;
};

struct RHI_COMMAND_QUEUE: public RHI_OBJECT {
	RHI_COMMAND_QUEUE(RHI_NATIVE_HANDLE* ptr, std::unique_ptr<RHI_OBJECT>&& queue_fence)
		: RHI_OBJECT(ptr)
		, fence(std::move(queue_fence)) {}
	RHI_OBJECT& get_fence() {
		return *fence;
	}
protected:
	std::unique_ptr<RHI_OBJECT> fence;
};

struct RHI_COMMAND_BUUFER_LIST {
	RHI_COMMAND_BUUFER_LIST(RHI_COMMAND_QUEUE& cmd_queue,
		std::vector<RHI_OBJECT*>&& cmd_buffers)
	: command_queue(&cmd_queue)
	, command_buffers(std::move(cmd_buffers))
	, counter(1) {}
	std::vector<RHI_OBJECT*>& get_list() {
		return command_buffers;
	}
	void increment_counter() {
		counter++;
	}
	unsigned __int64 get_counter() const {
		return counter;
	}
	RHI_COMMAND_QUEUE& get_queue() {
		return *command_queue;
	}
private:
	std::vector<RHI_OBJECT*>&& command_buffers;
	unsigned __int64 counter;
	RHI_COMMAND_QUEUE* command_queue;
};

struct RHI_DEVICE_DESC {
	int adapter_id = -1;
	unsigned long long features = device_features_none;
};

struct RHI_BUFFER_DESC {
	RHI_OBJECT* device;
	size_t size;
	buffer_memory_type memory_type;
	resource_state initial_state;
	bool is_uav;
};

struct RHI_VERTEX_BUFFER_DESC : public RHI_BUFFER_DESC {
	char name[_MAX_FNAME];
	vertex_format format;
	size_t stride;
	size_t count;
};

struct RHI_INDEX_BUFFER_DESC : public RHI_BUFFER_DESC {
	resource_format format;
	size_t count;
};

struct RHI_WINDOW_DESC {
	char title[_MAX_FNAME];
	size_t width;
	size_t height;
	bool full_screen;
};

struct RHI_COMMAND_QUEUE_DESC {
	RHI_OBJECT* device;
};

struct RHI_SWAP_CHAIN_DESC {
	size_t width;
	size_t height;
	resource_format color_format;
	bool allow_tearing;
	RHI_NATIVE_HANDLE* window;
	RHI_OBJECT* device;
	RHI_OBJECT* command_queue;
	size_t buffer_count;
};

struct RHI_COMMAND_BUFFER_DESC {
	RHI_OBJECT* device;
	RHI_OBJECT* command_queue;
	queue_type type;
};

struct RHI_RASTER_PIPELINE_DESC {
	RHI_OBJECT* device;
	primitive_topology topology = primitive_topology_none;
};

struct RHI_RT_PIPELINE_DESC {
	RHI_OBJECT* device;
};

struct RHI_TEXTURE_2D_DESC {
	RHI_OBJECT* device;
	size_t width;
	size_t height;
	resource_format format;
};

struct RHI_FENCE_DESC {
	RHI_OBJECT* device;
	fence_flags flags = fence_flags_none;
	int initial_value = 0;
};

struct RHI_RT_BVH_DESC {
	RHI_OBJECT* device;
	RHI_COMMAND_QUEUE* command_queue;
	RHI_OBJECT* command_buffer;
	RHI_GEOMETRY* geometry_buffer;
};

struct RHI_TRANSFER_BUFFER_DESC {
	RHI_OBJECT* device;
	RHI_RESOURCE* buffer;
	RHI_COMMAND_QUEUE* command_queue;
	RHI_OBJECT* command_list;
	void* data;
	size_t transfer_size;
};

void rhi_init(device_type dt);
void rhi_end();

inline std::unique_ptr<RHI_OBJECT>(*rhi_create_device)(const RHI_DEVICE_DESC& desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_swap_chain)(const RHI_SWAP_CHAIN_DESC& swpc_desc);
inline std::unique_ptr<RHI_NATIVE_HANDLE>(*rhi_create_window)(const RHI_WINDOW_DESC& desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_graphics_command_queue)(const RHI_COMMAND_QUEUE_DESC& queue_desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_compute_command_queue)(const RHI_COMMAND_QUEUE_DESC& queue_desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_transfer_command_queue)(const RHI_COMMAND_QUEUE_DESC& queue_desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_command_buffer)(const RHI_COMMAND_BUFFER_DESC& cb_desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_pipeline)(const RHI_RASTER_PIPELINE_DESC& pipeline_desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_vertex_buffer)(const RHI_VERTEX_BUFFER_DESC& vb_desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_index_buffer)(const RHI_INDEX_BUFFER_DESC& ib_desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_texture_2d)(const RHI_TEXTURE_2D_DESC& tex_desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_fence)(const RHI_FENCE_DESC& desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_raster_pipeline)(const RHI_RASTER_PIPELINE_DESC& desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_rt_pipeline)(const RHI_RT_PIPELINE_DESC& desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_compile_shader)(const char* const file,	const char* const entry, const char* const target);
#endif