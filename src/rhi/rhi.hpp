#ifndef __rhi_hpp__
#define __rhi_hpp__

#include "strings.hpp"
#include "file_system.hpp"
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

struct RHI_OBJECT;
using fptr_window_main_loop_callback = std::function<void()>;
using fptr_window_on_init = std::function<void(RHI_OBJECT& window_handle)>;
using fptr_window_on_end = std::function<void()>;

struct RHI_WINDOW_CALLBACKS {
	
	fptr_window_on_init on_init;
	fptr_window_main_loop_callback main_loop;
	fptr_window_on_end on_end;
};

struct RHI_NATIVE_HANDLE {
	RHI_NATIVE_HANDLE(void* h) : handle(h) {}
	virtual ~RHI_NATIVE_HANDLE() = default;
	void* get_handle() { return handle; }
private:
	void* handle;
};

struct DefaultDeleter { void operator()(void*) const {} };
template<typename I, typename U = DefaultDeleter>
struct RHI_TEMPLATE_HANDLE : public RHI_NATIVE_HANDLE {

	RHI_TEMPLATE_HANDLE(I* i) 
		: template_instance(i)
		, RHI_NATIVE_HANDLE(&template_instance) {
	}
	~RHI_TEMPLATE_HANDLE() {
		printf("deleting %s\n", get_type_name<I>().c_str());
	}
	operator I* () { 
		return template_instance.get();
	}
private:
	std::unique_ptr<I, U> template_instance;
};

template<typename T>
struct RHI_WINDOW_HANDLE : public RHI_NATIVE_HANDLE {
	RHI_WINDOW_HANDLE(T h, const RHI_WINDOW_CALLBACKS& window_callbacks)
		: RHI_NATIVE_HANDLE(h)
		, callbacks(window_callbacks) {
	}
	operator T() { return reinterpret_cast<T>(this->get_handle()); }
	const RHI_WINDOW_CALLBACKS& get_callbacks() const { return callbacks; }
private:
	const RHI_WINDOW_CALLBACKS& callbacks;
};

struct RHI_OBJECT {
	virtual ~RHI_OBJECT() = default;
	RHI_OBJECT(RHI_NATIVE_HANDLE* ptr) {
		native_impl.reset(ptr);
	}
	RHI_OBJECT& operator()() { return *this; }
	operator RHI_NATIVE_HANDLE& () {
		return *native_impl.get();
	}
	template<typename T>
	T& handle() {

		return static_cast<T&>(static_cast<RHI_NATIVE_HANDLE&>(*this));
	}
private:
	std::unique_ptr<RHI_NATIVE_HANDLE> native_impl;
};

struct RHI_RESOURCE : public RHI_OBJECT {
	RHI_RESOURCE(RHI_NATIVE_HANDLE* ptr)
		: RHI_OBJECT(ptr) {}
	resource_state get_current_state() const {
		return current_state;
	}
protected:
	void set_current_state(resource_state state) {
		current_state = state;
	}	
private:
	resource_state current_state = resource_state_none;
	resource_format format = resource_format_none;
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
		: vbo(vb)
		, ibo(ib) {}
	RHI_RESOURCE& get_vertex_buffer() {
		return vbo.get();
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
	std::reference_wrapper<RHI_RESOURCE> vbo;
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

struct RHI_RENDER_PASS : public RHI_OBJECT {
	RHI_RENDER_PASS(RHI_NATIVE_HANDLE* ptr, RHI_OBJECT& dev, RHI_RESOURCE& buffer)
		: RHI_OBJECT(ptr)
		, device(dev)
		, surface(buffer) {}
		
	RHI_OBJECT& get_device() { return device; }
	RHI_OBJECT& get_surface() { return surface; }
private:
	std::reference_wrapper<RHI_RESOURCE> surface;
	std::reference_wrapper<RHI_OBJECT> device;
};

struct RHI_COMMAND_BUUFER_LIST {
	RHI_COMMAND_BUUFER_LIST(RHI_OBJECT& cmd_queue,
		std::vector<RHI_OBJECT*>&& cmd_buffers)
	: command_queue(cmd_queue)
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
	RHI_OBJECT& get_queue() {
		return command_queue.get();
	}
private:
	std::reference_wrapper<RHI_OBJECT> command_queue;
	std::vector<RHI_OBJECT*>&& command_buffers;
	unsigned __int64 counter;
};

struct RHI_DEVICE_DESC {
	int adapter_id = -1;
	unsigned long long features = device_features_none;
};

struct RHI_BUFFER_DESC {

	RHI_BUFFER_DESC(RHI_OBJECT& dev)
		: device(dev)
		, size(0)
		, memory_type(buffer_memory_type_gpu_only)
	    , initial_state(resource_state_none)
	    , is_uav(false) {}
	std::reference_wrapper<RHI_OBJECT> device;
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
	RHI_WINDOW_CALLBACKS callbacks;
};

struct RHI_COMMAND_QUEUE_DESC {
	RHI_COMMAND_QUEUE_DESC(RHI_OBJECT& dev)
		: device(dev) {}
	std::reference_wrapper<RHI_OBJECT> device;
};

struct RHI_SWAP_CHAIN_DESC {

	RHI_SWAP_CHAIN_DESC(RHI_OBJECT& dev, RHI_OBJECT& cmd_queue, RHI_OBJECT& wnd)
		: device(dev)
		, command_queue(cmd_queue)
		, window(wnd)
		, width(0)
		, height(0)
		, color_format(resource_format_none)
		, allow_tearing(false)
		, buffer_count(0) {}	

	std::reference_wrapper<RHI_OBJECT> device;
	std::reference_wrapper<RHI_OBJECT> command_queue;
	std::reference_wrapper<RHI_OBJECT> window;
	size_t width;
	size_t height;
	resource_format color_format;
	bool allow_tearing;
	size_t buffer_count;
};

struct RHI_COMMAND_BUFFER_DESC {

	RHI_COMMAND_BUFFER_DESC(RHI_OBJECT& dev, RHI_OBJECT& cmd_queue)
		: device(dev)
		, command_queue(cmd_queue)
		, type(queue_type_graphics) { }

	std::reference_wrapper<RHI_OBJECT> device;
	std::reference_wrapper<RHI_OBJECT> command_queue;
	queue_type type;
};

struct RHI_RASTER_PIPELINE_DESC {

	RHI_RASTER_PIPELINE_DESC(RHI_OBJECT& dev)
		: device(dev)
		, topology(primitive_topology_none) {}

	std::reference_wrapper<RHI_OBJECT> device;
	primitive_topology topology;
};

struct RHI_RT_PIPELINE_DESC {

	RHI_RT_PIPELINE_DESC(RHI_OBJECT& dev)
		: device(dev) {}
	std::reference_wrapper<RHI_OBJECT> device;
};

struct RHI_TEXTURE_2D_DESC {

	RHI_TEXTURE_2D_DESC(RHI_OBJECT& dev)
		: device(dev)
		, width(0)
		, height(0)
		, format(resource_format_none) {}
	std::reference_wrapper<RHI_OBJECT> device;
	size_t width;
	size_t height;
	resource_format format;
};

struct RHI_FENCE_DESC {

	RHI_FENCE_DESC(RHI_OBJECT& dev)
		: device(dev)
		, flags(fence_flags_none)
		, initial_value(0) {}
	std::reference_wrapper<RHI_OBJECT> device;
	fence_flags flags;
	int initial_value;
};

struct RHI_RT_BVH_DESC {

	RHI_RT_BVH_DESC(RHI_OBJECT& dev, RHI_COMMAND_QUEUE& cmd_queue,
					RHI_OBJECT& cmd_buffer, RHI_GEOMETRY& geo_buffer)
		: device(dev)
		, command_queue(cmd_queue)
		, command_buffer(cmd_buffer)
		, geometry_buffer(geo_buffer) {}
	std::reference_wrapper<RHI_OBJECT> device;
	std::reference_wrapper<RHI_COMMAND_QUEUE> command_queue;
	std::reference_wrapper<RHI_OBJECT> command_buffer;
	std::reference_wrapper<RHI_GEOMETRY> geometry_buffer;
};

struct RHI_TRANSFER_BUFFER_DESC {

	RHI_TRANSFER_BUFFER_DESC(RHI_OBJECT& dev, RHI_COMMAND_QUEUE& cmd_queue,
		RHI_OBJECT& cmd_buffer, RHI_RESOURCE& buff)
		: device(dev)
		, command_queue(cmd_queue)
		, command_buffer(cmd_buffer)
		, buffer(buff)
		, data(nullptr)
		, transfer_size(0) {
	}

	std::reference_wrapper<RHI_OBJECT> device;
	std::reference_wrapper<RHI_COMMAND_QUEUE> command_queue;
	std::reference_wrapper<RHI_OBJECT> command_buffer;
	std::reference_wrapper<RHI_RESOURCE> buffer;	
	void* data;
	size_t transfer_size;
};

struct RHI_RENDER_PASS_DESC {

	RHI_RENDER_PASS_DESC(RHI_OBJECT& dev, RHI_RESOURCE& buffer)
		: device(dev)
		, surface(buffer)
		, format(resource_format_none)
		, buffer_index(-1) {
	}
	std::reference_wrapper<RHI_OBJECT> device;
	std::reference_wrapper<RHI_RESOURCE> surface;
	resource_format format;
	int buffer_index;
};

void rhi_init(device_type dt);
void rhi_end();

inline std::unique_ptr<RHI_OBJECT>(*rhi_create_window)(const RHI_WINDOW_DESC& desc);
inline void(*rhi_window_main_loop)(RHI_OBJECT& handle);

inline std::unique_ptr<RHI_OBJECT>(*rhi_create_device)(const RHI_DEVICE_DESC& desc);

// swap chain api
inline std::unique_ptr<RHI_OBJECT>(*rhi_create_swap_chain)(const RHI_SWAP_CHAIN_DESC& swpc_desc);
inline void (*rhi_swap_chain_present)(RHI_OBJECT& swap_chain);

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