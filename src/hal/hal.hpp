#ifndef __hal_hpp__
#define __hal_hpp__

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

struct HAL_HANDLE {
	virtual ~HAL_HANDLE() = default;
	virtual void* get_native_handle() = 0;
};

struct HAL_OBJECT {
	virtual ~HAL_OBJECT() = default;
	HAL_OBJECT(HAL_HANDLE* ptr) {
		native_impl.reset(ptr);
	}
	std::unique_ptr<HAL_HANDLE>& get_native_impl() {
		return native_impl;
	}
	
private:
	std::unique_ptr<HAL_HANDLE> native_impl;
};

struct HAL_RESOURCE : public HAL_OBJECT {
	HAL_RESOURCE(HAL_HANDLE* ptr)
		: HAL_OBJECT(ptr)
		, command_buffer(nullptr) {}
	resource_state get_current_state() const {
		return current_state;
	}
	void set_command_buffer(HAL_OBJECT* cmd_buffer) {
		command_buffer = cmd_buffer;
	}
	HAL_OBJECT* get_command_buffer() {
		return command_buffer;
	}
	virtual void change_state(resource_state new_state) = 0;
protected:
	void set_current_state(resource_state state) {
		current_state = state;
	}	
private:
	resource_state current_state = resource_state_none;
	HAL_OBJECT* command_buffer;
};

struct HAL_COMMAND_QUEUE: public HAL_OBJECT {
	HAL_COMMAND_QUEUE(HAL_HANDLE* ptr, std::unique_ptr<HAL_OBJECT>&& queue_fence)
		: HAL_OBJECT(ptr)
		, fence(std::move(queue_fence)) {}
	HAL_OBJECT& get_fence() {
		return *fence;
	}
protected:
	std::unique_ptr<HAL_OBJECT> fence;
};

struct HAL_COMMAND_BUUFER_LIST {
	HAL_COMMAND_BUUFER_LIST(HAL_COMMAND_QUEUE& cmd_queue,
		std::vector<std::unique_ptr<HAL_OBJECT>>&& cmd_buffers)
	: command_queue(&cmd_queue)
	, command_buffers(std::move(cmd_buffers))
	, counter(1) {}
	std::vector<std::unique_ptr<HAL_OBJECT>>& get_list() {
		return command_buffers;
	}
	void increment_counter() {
		counter++;
	}
	unsigned __int64 get_counter() const {
		return counter;
	}
	HAL_COMMAND_QUEUE& get_queue() {
		return *command_queue;
	}
private:
	std::vector<std::unique_ptr<HAL_OBJECT>>&& command_buffers;
	unsigned __int64 counter;
	HAL_COMMAND_QUEUE* command_queue;
};

struct HAL_DEVICE_DESC {
	int adapter_id = -1;
	unsigned long long features = device_features_none;
};

struct HAL_BUFFER_DESC {
	size_t size;
};

struct HAL_VERTEX_BUFFER_DESC : public HAL_BUFFER_DESC {
	HAL_OBJECT* device;
	char name[_MAX_FNAME];
	vertex_format format;
	size_t stride;
	size_t count;
};

struct HAL_INDEX_BUFFER_DESC : public HAL_BUFFER_DESC {
	HAL_OBJECT* device;
	index_format format;
	size_t count;
};

struct HAL_WINDOW_DESC {
	char title[_MAX_FNAME];
	size_t width;
	size_t height;
	bool full_screen;
};

struct HAL_COMMAND_QUEUE_DESC {
	HAL_OBJECT* device;
};

struct HAL_SWAP_CHAIN_DESC {
	size_t width;
	size_t height;
	color_format color_format;
	bool allow_tearing;
	HAL_HANDLE* window;
	HAL_OBJECT* device;
	HAL_OBJECT* command_queue;
	size_t buffer_count;
};

struct HAL_COMMAND_BUFFER_DESC {
	HAL_OBJECT* device;
	HAL_OBJECT* command_queue;
	queue_type type;
};

struct HAL_PIPELINE_DESC {
	HAL_OBJECT* device;
	primitive_topology topology = primitive_topology_none;
};

struct HAL_TEXTURE_2D_DESC {
	HAL_OBJECT* device;
	size_t width;
	size_t height;
	color_format format;
};

struct HAL_FENCE_DESC {
	HAL_OBJECT* device;
	fence_flags flags = fence_flags_none;
	int initial_value = 0;
};

void hal_init(device_type dt);
void hal_end();

inline std::unique_ptr<HAL_OBJECT>(*hal_create_device)(const HAL_DEVICE_DESC& desc);
inline std::unique_ptr<HAL_OBJECT>(*hal_create_swap_chain)(const HAL_SWAP_CHAIN_DESC& swpc_desc);
inline std::unique_ptr<HAL_HANDLE>(*hal_create_window)(const HAL_WINDOW_DESC& desc);
inline std::unique_ptr<HAL_OBJECT>(*hal_create_graphics_command_queue)(const HAL_COMMAND_QUEUE_DESC& queue_desc);
inline std::unique_ptr<HAL_OBJECT>(*hal_create_compute_command_queue)(const HAL_COMMAND_QUEUE_DESC& queue_desc);
inline std::unique_ptr<HAL_OBJECT>(*hal_create_transfer_command_queue)(const HAL_COMMAND_QUEUE_DESC& queue_desc);
inline std::unique_ptr<HAL_OBJECT>(*hal_create_command_buffer)(const HAL_COMMAND_BUFFER_DESC& cb_desc);
inline std::unique_ptr<HAL_OBJECT>(*hal_create_pipeline)(const HAL_PIPELINE_DESC& pipeline_desc);
inline std::unique_ptr<HAL_OBJECT>(*hal_create_vertex_buffer)(const HAL_VERTEX_BUFFER_DESC& vb_desc);
inline std::unique_ptr<HAL_OBJECT>(*hal_create_index_buffer)(const HAL_INDEX_BUFFER_DESC& ib_desc);
inline std::unique_ptr<HAL_OBJECT>(*hal_create_texture_2d)(const HAL_TEXTURE_2D_DESC& tex_desc);
inline std::unique_ptr<HAL_OBJECT>(*hal_create_fence)(const HAL_FENCE_DESC& desc);

#endif