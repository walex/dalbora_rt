#ifndef __rhi_hpp__
#define __rhi_hpp__

#include <memory>
#include <vector>

enum device_type {
	device_type_none = 0,
	device_type_dx12
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

enum index_format {
	index_format_None = 0,
	index_format_UInt16 = 16,
	index_format_UInt32 = 32
};

enum primitive_topology {
	primitive_topology_none = 0,
	primitive_topology_point,
	primitive_topology_line,
	primitive_topology_triangle,
	primitive_topology_patch
};

enum color_format {
	color_format_none = 0,
	color_format_R8G8B8A8 = 1
};

enum resource_state {
	resource_state_none = 0,
	resource_state_render_target = 1,
	resource_state_depth_read = 2,
	resource_state_depth_write = 3,
	resource_state_shader_write = 4,
	resource_state_shader_read = 5,
	resource_state_copy_src = 6,
	resource_state_copy_dest = 7,
	resource_state_present = 8,
	resource_state_rt_bvh = 9
};

enum fence_flags {
	fence_flags_none = 0,
	fence_flags_shared = 1
};

constexpr __int64 device_features_none = 0x0;
constexpr __int64 device_features_raytracing = 0x1;
constexpr __int64 device_features_variable_rate_shading = 0x2;
constexpr __int64 device_features_mesh_shaders = 0x4;

struct RHI_HANDLE {
	virtual ~RHI_HANDLE() = default;
	virtual void* get_native_handle() = 0;
};

struct RHI_OBJECT {
	virtual ~RHI_OBJECT() = default;
	RHI_OBJECT(RHI_HANDLE* ptr) {
		native_impl.reset(ptr);
	}
	std::unique_ptr<RHI_HANDLE>& get_native_impl() {
		return native_impl;
	}
	
private:
	std::unique_ptr<RHI_HANDLE> native_impl;
};

struct RHI_RESOURCE : public RHI_OBJECT {
	RHI_RESOURCE(RHI_HANDLE* ptr)
		: RHI_OBJECT(ptr)
		, command_buffer(nullptr) {}
	resource_state get_current_state() const {
		return current_state;
	}
	void set_command_buffer(RHI_OBJECT* cmd_buffer) {
		command_buffer = cmd_buffer;
	}
	RHI_OBJECT* get_command_buffer() {
		return command_buffer;
	}
	virtual void change_state(resource_state new_state) = 0;
protected:
	void set_current_state(resource_state state) {
		current_state = state;
	}	
private:
	resource_state current_state = resource_state_none;
	RHI_OBJECT* command_buffer;
};

struct RHI_COMMAND_QUEUE: public RHI_OBJECT {
	RHI_COMMAND_QUEUE(RHI_HANDLE* ptr, std::unique_ptr<RHI_OBJECT>&& queue_fence)
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
		std::vector<std::unique_ptr<RHI_OBJECT>>&& cmd_buffers)
	: command_queue(&cmd_queue)
	, command_buffers(std::move(cmd_buffers))
	, counter(1) {}
	std::vector<std::unique_ptr<RHI_OBJECT>>& get_list() {
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
	std::vector<std::unique_ptr<RHI_OBJECT>>&& command_buffers;
	unsigned __int64 counter;
	RHI_COMMAND_QUEUE* command_queue;
};

struct RHI_DEVICE_DESC {
	int adapter_id = -1;
	unsigned long long features = device_features_none;
};

struct RHI_BUFFER_DESC {
	size_t size;
};

struct RHI_VERTEX_BUFFER_DESC : public RHI_BUFFER_DESC {
	RHI_OBJECT* device;
	char name[_MAX_FNAME];
	vertex_format format;
	size_t stride;
	size_t count;
};

struct RHI_INDEX_BUFFER_DESC : public RHI_BUFFER_DESC {
	RHI_OBJECT* device;
	index_format format;
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
	color_format color_format;
	bool allow_tearing;
	RHI_HANDLE* window;
	RHI_OBJECT* device;
	RHI_OBJECT* command_queue;
	size_t buffer_count;
};

struct RHI_COMMAND_BUFFER_DESC {
	RHI_OBJECT* device;
	RHI_OBJECT* command_queue;
	queue_type type;
};

struct RHI_PIPELINE_DESC {
	RHI_OBJECT* device;
	primitive_topology topology = primitive_topology_none;
};

struct RHI_TEXTURE_2D_DESC {
	RHI_OBJECT* device;
	size_t width;
	size_t height;
	color_format format;
};

struct RHI_FENCE_DESC {
	RHI_OBJECT* device;
	fence_flags flags = fence_flags_none;
	int initial_value = 0;
};

void rhi_init(device_type dt);
void rhi_end();

inline std::unique_ptr<RHI_OBJECT>(*rhi_create_device)(const RHI_DEVICE_DESC& desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_swap_chain)(const RHI_SWAP_CHAIN_DESC& swpc_desc);
inline std::unique_ptr<RHI_HANDLE>(*rhi_create_window)(const RHI_WINDOW_DESC& desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_graphics_command_queue)(const RHI_COMMAND_QUEUE_DESC& queue_desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_compute_command_queue)(const RHI_COMMAND_QUEUE_DESC& queue_desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_transfer_command_queue)(const RHI_COMMAND_QUEUE_DESC& queue_desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_command_buffer)(const RHI_COMMAND_BUFFER_DESC& cb_desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_pipeline)(const RHI_PIPELINE_DESC& pipeline_desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_vertex_buffer)(const RHI_VERTEX_BUFFER_DESC& vb_desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_index_buffer)(const RHI_INDEX_BUFFER_DESC& ib_desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_texture_2d)(const RHI_TEXTURE_2D_DESC& tex_desc);
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_fence)(const RHI_FENCE_DESC& desc);

#endif