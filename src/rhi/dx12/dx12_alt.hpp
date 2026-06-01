#ifndef __dx12_alt_hpp__
#define __dx12_alt_hpp__

#include "platform.hpp"
#include "dx12_rhi.hpp"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <wrl.h>

#define RHI_STRUCT_BASE_PARAMS(...) __VA_ARGS__
#define DEFINE_SETTER(var_name) \
	void set_##var_name(auto value) { var_name = value; }
#define DEFINE_GETTER(var_name) \
	auto get_##var_name() const { return var_name; }

#define RHI_STRUCT_BASE_INFO(name, ...)                                \
	virtual ~name() = default;                                         \
	template <typename T>                                              \
	operator T() { return static_cast<T>(this->get_native_handle()); } \
                                                                       \
                                                            \
	virtual RHI_VOID_PTR get_native_handle() = 0;                      \
name __VA_ARGS__ public:

#define HANDLE_CONSTRUCTOR(name, iface, ...) \
	name(iface *ptr) : name##_HANDLE(ptr) { __VA_ARGS__ }
#define IMPLEMENT_GET_NATIVE_HANDLE(iface) \
	RHI_VOID_PTR get_native_handle() override { return static_cast<iface *>(*this); }

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

enum pipeline_type
{
	pipeline_type_raster = 0,
	pipeline_type_rt
};

using RHI_SHADER_TABLE_ENTIRES2 = std::map<std::string, RHI_VOID_PTR>;


struct RHI_PLATFORM_DESC2
{
	RHI_VOID_PTR platform_desc_ptr = nullptr;
};

struct RHI_DEVICE2 {
	RHI_STRUCT_BASE_INFO(RHI_DEVICE2, () {})
};
struct RHI_GRAPHICS_PIPELINE2 {
	RHI_STRUCT_BASE_INFO(RHI_GRAPHICS_PIPELINE2, RHI_STRUCT_BASE_PARAMS(

		(pipeline_type type) : type(type) {
	}
	))
		pipeline_type get_type() { return type; }
private:
	pipeline_type type;
};

struct RHI_PIPELINE_LAYOUT2 {
	RHI_STRUCT_BASE_INFO(RHI_PIPELINE_LAYOUT2, () {})
};

struct RHI_RASTER_PIPELINE2 : public RHI_GRAPHICS_PIPELINE2 {
	RHI_RASTER_PIPELINE2(RHI_PIPELINE_LAYOUT2& layout)
		: RHI_GRAPHICS_PIPELINE2(pipeline_type_raster)
		, layout(layout) {
	}
	operator RHI_PIPELINE_LAYOUT2& () { return layout.get(); }
private:
	std::reference_wrapper<RHI_PIPELINE_LAYOUT2> layout;
};

struct RHI_RT_PIPELINE2 : public RHI_GRAPHICS_PIPELINE2 {
	RHI_RT_PIPELINE2(RHI_PIPELINE_LAYOUT2& layout, std::unique_ptr<RHI_SHADER_TABLE_ENTIRES2> sbt)
		: RHI_GRAPHICS_PIPELINE2(pipeline_type_rt)
		, layout(layout)
		, shader_table(std::move(sbt)) {
	}
	operator RHI_PIPELINE_LAYOUT2& () { return layout.get(); }
	operator RHI_SHADER_TABLE_ENTIRES2& () { return  *shader_table; }

	std::reference_wrapper<RHI_PIPELINE_LAYOUT2> layout;
	std::unique_ptr<RHI_SHADER_TABLE_ENTIRES2> shader_table;
};

struct RHI_RESOURCE2 {
	RHI_STRUCT_BASE_INFO(RHI_RESOURCE2, RHI_STRUCT_BASE_PARAMS(

		(resource_state default_state, resource_format resource_format)
		: current_state(default_state), default_state(default_state), format(resource_format) {
	}
	))
public:
	DEFINE_SETTER(current_state);
	DEFINE_GETTER(current_state);
	DEFINE_GETTER(default_state);
	DEFINE_GETTER(format);

	resource_state current_state;
	resource_state default_state;
	resource_format format;
};

struct RHI_BUFFER2 : public RHI_RESOURCE2 {

	RHI_STRUCT_BASE_INFO(RHI_BUFFER2, RHI_STRUCT_BASE_PARAMS(

		(resource_state default_state, resource_format resource_format, size_t length)
		: RHI_RESOURCE2(default_state, resource_format)
		, length(length) {
	}
	))
public:
	DEFINE_SETTER(length);
	DEFINE_GETTER(length);
private:
	size_t length;
};

struct RHI_TEXTURE_2D2 : public RHI_BUFFER2 {

	RHI_TEXTURE_2D2(resource_state base_state, resource_format resource_format,
		size_t width, size_t height, size_t phisycal_size,
		std::vector<RHI_TEXTURE_MIPS>&& mips)
		: RHI_BUFFER2(base_state, resource_format, phisycal_size)
		, width(width)
		, height(height)
		, mipmaps(std::move(mips)) {
	}
	std::vector<RHI_TEXTURE_MIPS>& get_mips() { return mipmaps; };
	DEFINE_GETTER(width);
	DEFINE_GETTER(height);

	size_t width, height;
	std::vector<RHI_TEXTURE_MIPS> mipmaps;
};

struct RHI_CONSTANT_BUFFER2 : public RHI_BUFFER2 {

	RHI_CONSTANT_BUFFER2(resource_state base_state, resource_format resource_format,
		size_t length) : RHI_BUFFER2(base_state, resource_format, length) {
	}
};

struct RHI_SWAP_CHAIN2 {
	RHI_STRUCT_BASE_INFO(RHI_SWAP_CHAIN2, () {})
		void add_render_target(std::shared_ptr<RHI_TEXTURE_2D2> rt) {
		m_render_targets.push_back(rt);
	}
	std::shared_ptr<RHI_TEXTURE_2D2> get_render_target(size_t index) {

		return m_render_targets.at(index);
	}
	size_t get_render_target_count() { return m_render_targets.size(); }

	std::vector<std::shared_ptr<RHI_TEXTURE_2D2>> m_render_targets;
};

struct RHI_DEPTH_BUFFER2 : public RHI_BUFFER2 {

	RHI_DEPTH_BUFFER2(resource_state base_state, resource_format resource_format,
		size_t width, size_t height) : RHI_BUFFER2(base_state, resource_format, width* height) {
	}
	DEFINE_GETTER(width);
	DEFINE_GETTER(height);
private:
	size_t width, height;
};


struct RHI_RENDER_PASS2 {

	RHI_STRUCT_BASE_INFO(RHI_RENDER_PASS2, RHI_STRUCT_BASE_PARAMS(

		(RHI_DEVICE2& device, std::shared_ptr<RHI_TEXTURE_2D2> rt)
		: device(device), render_target(rt), depth_buffer(nullptr) {
	}
	))

	operator RHI_DEVICE2& () { return device.get(); }
	operator RHI_TEXTURE_2D2& () { return *render_target; }
	operator RHI_DEPTH_BUFFER2* () { return depth_buffer; }
	operator RHI_GRAPHICS_PIPELINE2* () { return pipeline; }
	operator RHI_VIEWPORT& () { return view_port; }
	operator std::vector<RHI_CONSTANT_BUFFER2*>& () { return constant_buffers; }
	void set_depth_buffer(RHI_DEPTH_BUFFER2* db) { depth_buffer = db; }
	void set_pipeline(RHI_GRAPHICS_PIPELINE2* pl) { pipeline = pl; }
	void set_view_port(const RHI_VIEWPORT& vp) { view_port = vp; }

	std::reference_wrapper<RHI_DEVICE2> device;
	std::shared_ptr<RHI_TEXTURE_2D2> render_target;
	RHI_DEPTH_BUFFER2* depth_buffer;
	RHI_GRAPHICS_PIPELINE2* pipeline;
	std::vector<RHI_CONSTANT_BUFFER2*> constant_buffers;
	RHI_VIEWPORT view_port;
};

struct RHI_RT_SHADER_UNIT2 {

	std::string name_id;	// id
	std::unique_ptr<RHI_COMPILED_SHADER_BUFFER> blob;				// blob buffer
};

struct RHI_RT_BVH2 {

	virtual ~RHI_RT_BVH2() = default;
};



struct RHI_VERTEX_BUFFER2 : public RHI_BUFFER2 {

	RHI_VERTEX_BUFFER2(resource_state base_state, resource_format resource_format,
		size_t length, size_t stride)
		: RHI_BUFFER2(base_state, resource_format, length)
		, stride(stride) {
	}
	DEFINE_GETTER(stride);
private:
	size_t stride;
};

struct RHI_INDEX_BUFFER2 : public RHI_BUFFER2 {

	RHI_INDEX_BUFFER2(resource_state base_state, resource_format resource_format,
		size_t length, size_t stride)
		: RHI_BUFFER2(base_state, resource_format, length)
		, stride(stride) {
	}
	DEFINE_GETTER(stride);
private:
	size_t stride;
};

// ---- DX_ 
template <typename T>
struct DX_NATIVE_HANDLE2 : public Microsoft::WRL::ComPtr<T>
{

	DX_NATIVE_HANDLE2(T* ptr) : Microsoft::WRL::ComPtr<T>(ptr) {}
	virtual ~DX_NATIVE_HANDLE2() = default;
	operator T* () { return this->Get(); }
};

using DX_RASTER_PIPELINE2_HANDLE = DX_NATIVE_HANDLE2<ID3D12PipelineState>;
using DX_RT_PIPELINE2_HANDLE = DX_NATIVE_HANDLE2<ID3D12StateObject>;

using DX_RESOURCE2_HANDLE = DX_NATIVE_HANDLE2<ID3D12Resource>;
struct DX_RESOURCE2 : public DX_RESOURCE2_HANDLE
{

	HANDLE_CONSTRUCTOR(DX_RESOURCE2, ID3D12Resource)
};

struct DX_RT_BVH2 : public RHI_RT_BVH2, public DX_RESOURCE2
{

	DX_RT_BVH2(ID3D12Resource* blas_buffer)
		: DX_RESOURCE2(blas_buffer)
	{
	}
};

struct DX_RESOURCE_DESCRIPTOR2 : public DX_RESOURCE2
{

	DX_RESOURCE_DESCRIPTOR2(ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle)
		: DX_RESOURCE2(resource), cpu_handle(cpu_handle)
	{
	}
	operator D3D12_CPU_DESCRIPTOR_HANDLE& ()
	{
		return cpu_handle;
	}

private:
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
};

struct DX_CONSTANT_BUFFER2 : public DX_RESOURCE_DESCRIPTOR2, public RHI_CONSTANT_BUFFER2
{

	DX_CONSTANT_BUFFER2(ID3D12Resource* i_depth_buffer, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
		resource_state base_state, resource_format resource_format,
		size_t length)
		: DX_RESOURCE_DESCRIPTOR2(i_depth_buffer, cpu_handle), RHI_CONSTANT_BUFFER2(base_state, resource_format, length)
	{
	}

	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Resource)
};

using DX_PIPELINE_LAYOUT2_HANDLE = DX_NATIVE_HANDLE2<ID3D12RootSignature>;
struct DX_PIPELINE_LAYOUT2 : public RHI_PIPELINE_LAYOUT2, public DX_PIPELINE_LAYOUT2_HANDLE
{

	HANDLE_CONSTRUCTOR(DX_PIPELINE_LAYOUT2, ID3D12RootSignature)
		IMPLEMENT_GET_NATIVE_HANDLE(ID3D12RootSignature)
};

struct DX_RASTER_PIPELINE2 : public RHI_RASTER_PIPELINE2, public DX_RASTER_PIPELINE2_HANDLE
{

	DX_RASTER_PIPELINE2(ID3D12PipelineState* i_pipelune_state, RHI_PIPELINE_LAYOUT2& layout)
		: DX_RASTER_PIPELINE2_HANDLE(i_pipelune_state), RHI_RASTER_PIPELINE2(layout) {
	}
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12PipelineState)


		operator DX_PIPELINE_LAYOUT2& () { return static_cast<DX_PIPELINE_LAYOUT2&>(static_cast<RHI_PIPELINE_LAYOUT2&>(*this)); }
};

struct DX_RT_PIPELINE2 : public RHI_RT_PIPELINE2, public DX_RT_PIPELINE2_HANDLE
{
	DX_RT_PIPELINE2(ID3D12StateObject* i_pipelune_state, RHI_PIPELINE_LAYOUT2& layout, std::unique_ptr<RHI_SHADER_TABLE_ENTIRES2> sbt)
		: DX_RT_PIPELINE2_HANDLE(i_pipelune_state)
		, RHI_RT_PIPELINE2(layout, std::move(sbt)) {
	}
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12StateObject)

		operator DX_PIPELINE_LAYOUT2& () { return static_cast<DX_PIPELINE_LAYOUT2&>(static_cast<RHI_PIPELINE_LAYOUT2&>(*this)); }
};

struct DX_BUFFER2 : public RHI_BUFFER2, public DX_RESOURCE2
{
	
	DX_BUFFER2(ID3D12Resource* resource,
		resource_state default_state,
		resource_format resource_format, size_t length)
		: DX_RESOURCE2(resource), RHI_BUFFER2(default_state, resource_format, length)
	{
	}
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Resource)
};

struct DX_DEPTH_BUFFER2 : public DX_RESOURCE_DESCRIPTOR2, public RHI_DEPTH_BUFFER2
{

	DX_DEPTH_BUFFER2(ID3D12Resource* i_depth_buffer, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
		resource_state base_state, resource_format resource_format,
		size_t width, size_t height)
		: DX_RESOURCE_DESCRIPTOR2(i_depth_buffer, cpu_handle), RHI_DEPTH_BUFFER2(base_state, resource_format, width, height) {
	}

	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Resource)
};

struct DX_SBT_BUFFER2 : public DX_BUFFER2
{

	DX_SBT_BUFFER2(ID3D12Resource* resource,
		resource_state base_state,
		resource_format resource_format, size_t length,
		size_t ray_gen_offset, size_t miss_offset, size_t hit_group_offset,
		size_t ray_gen_size, size_t miss_size, size_t hit_group_size,
		size_t record_size)
		: DX_BUFFER2(resource, base_state, resource_format, length)
		, ray_gen_offset(ray_gen_offset)
		, miss_offset(miss_offset)
		, hit_group_offset(hit_group_offset)
		, ray_gen_size(ray_gen_size)
		, miss_size(miss_size)
		, hit_group_size(hit_group_size)
		, record_size(record_size)

	{
	}
	DEFINE_GETTER(ray_gen_offset)
		DEFINE_GETTER(miss_offset)
		DEFINE_GETTER(hit_group_offset)
		DEFINE_GETTER(ray_gen_size)
		DEFINE_GETTER(miss_size)
		DEFINE_GETTER(hit_group_size)
		DEFINE_GETTER(record_size)
private:
	size_t ray_gen_offset, miss_offset, hit_group_offset;
	size_t ray_gen_size, miss_size, hit_group_size;
	size_t record_size;
};

struct DX_BVH_BUFFER2 : public DX_RESOURCE_DESCRIPTOR2, public RHI_BUFFER2
{

	DX_BVH_BUFFER2(ID3D12Resource* i_buffer, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
		resource_state default_state, resource_format resource_format,
		size_t length)
		: DX_RESOURCE_DESCRIPTOR2(i_buffer, cpu_handle),
		RHI_BUFFER2(default_state, resource_format, length)
	{
	}
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Resource)
};


struct DX_TEXTURE_2D2 : public DX_RESOURCE_DESCRIPTOR2, public RHI_TEXTURE_2D2
{

	DX_TEXTURE_2D2(ID3D12Resource* i_buffer, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
		resource_state default_state, resource_format resource_format,
		size_t width, size_t height, size_t physical_size,
		std::vector<RHI_TEXTURE_MIPS>&& mips)
		: DX_RESOURCE_DESCRIPTOR2(i_buffer, cpu_handle), RHI_TEXTURE_2D2(default_state, resource_format, width, height, physical_size, std::move(mips))
	{
	}

	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Resource)
};


using DX_HEAP2_HANDLE = DX_NATIVE_HANDLE2<ID3D12DescriptorHeap>;
struct DX_HEAP2 : public DX_HEAP2_HANDLE
{

	HANDLE_CONSTRUCTOR(DX_HEAP2, ID3D12DescriptorHeap)
};

using DX_DEVICE2_HANDLE = DX_NATIVE_HANDLE2<ID3D12Device>;
struct DX_DEVICE2 : public RHI_DEVICE2, public DX_DEVICE2_HANDLE
{
	
	HANDLE_CONSTRUCTOR(DX_DEVICE2, ID3D12Device)
		IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Device)
		void set_rtv_heap(std::unique_ptr<DX_HEAP2>&& heap) { m_rtv_heap = std::move(heap); }
	DX_HEAP2* get_rtv_heap() { 
		return m_rtv_heap.get();
	}
	DX_HEAP2* get_dsv_heap() { 
		return m_dsv_heap.get();
	}
	DX_HEAP2* get_resources_heap() { 
		return m_resources_heap.get(); 
	}
	DX_HEAP2* get_sampler_heap() {
		return m_sampler_heap.get();
	}
	void set_dx_device(DX_DEVICE* dev) {
		device = dev;
		m_rtv_heap = std::make_unique<DX_HEAP2>(dev->rtv_heap->com_ptr.Get());
		m_dsv_heap = std::make_unique<DX_HEAP2>(dev->dsv_heap->com_ptr.Get());
		m_resources_heap = std::make_unique<DX_HEAP2>(dev->resources_heap->com_ptr.Get());
		//m_sampler_heap = std::make_unique<DX_HEAP2>(dev->sampler_heap->com_ptr.Get());

	}
private:
	DX_DEVICE* device;
	std::unique_ptr<DX_HEAP2> m_rtv_heap;
	std::unique_ptr<DX_HEAP2> m_dsv_heap;
	std::unique_ptr<DX_HEAP2> m_resources_heap;
	std::unique_ptr<DX_HEAP2> m_sampler_heap;
};
using DX_SWAP_CHAIN2_HANDLE = DX_NATIVE_HANDLE2<IDXGISwapChain3>;
struct DX_SWAP_CHAIN2 : public RHI_SWAP_CHAIN2, public DX_SWAP_CHAIN2_HANDLE
{

	HANDLE_CONSTRUCTOR(DX_SWAP_CHAIN2, IDXGISwapChain3)
		IMPLEMENT_GET_NATIVE_HANDLE(IDXGISwapChain3)
};

struct DX_INDEX_BUFFER2 : public RHI_INDEX_BUFFER2, public DX_RESOURCE2 {

	DX_INDEX_BUFFER2(ID3D12Resource* resource,
		resource_state base_state,
		resource_format resource_format, size_t length, size_t stride)
		: DX_RESOURCE2(resource)
		, RHI_INDEX_BUFFER2(base_state, resource_format, length, stride)
	{
	}
	DX_INDEX_BUFFER2* get_ib_ptr() { return this; }
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Resource)
};

struct DX_VERTEX_BUFFER2 : public RHI_VERTEX_BUFFER2, public DX_RESOURCE2 {

	DX_VERTEX_BUFFER2(ID3D12Resource* resource,
		resource_state base_state,
		resource_format resource_format, size_t length, size_t stride)
		: DX_RESOURCE2(resource)
		, RHI_VERTEX_BUFFER2(base_state, resource_format, length, stride)
	{
	}
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Resource)
};

struct DX_RENDER_PASS2 : public RHI_RENDER_PASS2
{

	DX_RENDER_PASS2(RHI_DEVICE2& device, std::shared_ptr<RHI_TEXTURE_2D2> rt)
		: RHI_RENDER_PASS2(device, rt)
	{
	}
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Device)
};

// --- DESC
struct RHI_BUFFER_DESC2 : public RHI_PLATFORM_DESC2
{

	RHI_BUFFER_DESC2(RHI_DEVICE2& dev)
		: device(dev)
		, length(0)
		, mips(1)
		, memory_type(buffer_memory_type_gpu_only)
		, default_state(resource_state_none)
		, format(resource_format_none)
		, type(buffer_type_raw)
	{
	}
	std::reference_wrapper<RHI_DEVICE2> device;
	size_t length, mips, resource_slot;
	buffer_memory_type memory_type;
	resource_state default_state;
	resource_format format;
	buffer_type type;
};

struct RHI_VERTEX_BUFFER_DESC2 : public RHI_BUFFER_DESC2
{

	RHI_VERTEX_BUFFER_DESC2(RHI_DEVICE2& device)
		: RHI_BUFFER_DESC2(device)
	{
	}
	size_t count;
	size_t stride;
};

struct RHI_INDEX_BUFFER_DESC2 : public RHI_BUFFER_DESC2
{

	RHI_INDEX_BUFFER_DESC2(RHI_DEVICE2& device)
		: RHI_BUFFER_DESC2(device)
	{
	}
	size_t count;
	size_t stride;
};

struct RHI_DESCRIPTOR_DESC2 : public RHI_PLATFORM_DESC2
{

	resource_type resource_type;
	int pool_range_start;
	int pool_range_count;
};

struct RHI_PIPELINE_LAYOUT_DESC2 : public RHI_PLATFORM_DESC2
{

	RHI_PIPELINE_LAYOUT_DESC2(RHI_DEVICE2& dev)
		: device(dev)
	{
	}

	std::reference_wrapper<RHI_DEVICE2> device;
	std::vector<RHI_DESCRIPTOR_DESC2> descriptors;
	raster_pipeline_shader_type shader_type;
};

struct RHI_RENDER_PASS_DESC2 : public RHI_PLATFORM_DESC2
{

	RHI_RENDER_PASS_DESC2(RHI_DEVICE2& device, std::shared_ptr<RHI_TEXTURE_2D2> buffer)
		: device(device), render_target(buffer), format(resource_format_none), synchronized(false)
	{
	}
	std::shared_ptr<RHI_TEXTURE_2D2> render_target;
	std::reference_wrapper<RHI_DEVICE2> device;
	resource_format format;
	bool synchronized;
};

struct RHI_BUFFER_2D_DESC2 : RHI_BUFFER_DESC2 {

	RHI_BUFFER_2D_DESC2(RHI_DEVICE2& device)
		: RHI_BUFFER_DESC2(device)
		, width(width)
		, height(height) {
	}
	size_t width;
	size_t height;
};

struct RHI_RT_SBT_DESC2 {

	std::vector<std::string> ray_gen_ids;
	std::vector<std::string> miss_ids;
	std::vector<std::string> hit_group_ids;
};

struct RHI_TEXTURE_2D_DESC2 : public RHI_BUFFER_2D_DESC2
{

	RHI_TEXTURE_2D_DESC2(RHI_DEVICE2& dev)
		: RHI_BUFFER_2D_DESC2(dev) {
	}

	size_t dims, depth;
	bool is_cube_map;
};

struct RHI_RT_BVH_DESC2 : public RHI_PLATFORM_DESC2
{

	RHI_RT_BVH_DESC2(RHI_DEVICE2& dev,
		RHI_COMMAND_BUFFER& cmd_buffer, RHI_BUFFER2& vertex_buffer,
		RHI_BUFFER2* index_buffer)
		: device(dev), command_buffer(cmd_buffer), vertex_buffer(vertex_buffer), index_buffer(index_buffer) {
	}
	std::reference_wrapper<RHI_DEVICE2> device;
	std::reference_wrapper<RHI_COMMAND_BUFFER> command_buffer;
	std::reference_wrapper<RHI_BUFFER2> vertex_buffer;
	RHI_BUFFER2* index_buffer;

};

struct RT_GEOMETRY_INSTANCES_DESC2 : public RHI_PLATFORM_DESC2
{

	RT_GEOMETRY_INSTANCES_DESC2(RHI_DEVICE2& dev,
		RHI_COMMAND_BUFFER& cmd_buffer, RHI_RT_BVH2& bvh)
		: device(dev), command_buffer(cmd_buffer), parent_bvh(bvh)
	{
	}
	std::reference_wrapper<RHI_DEVICE2> device;
	std::reference_wrapper<RHI_COMMAND_BUFFER> command_buffer;
	std::reference_wrapper<RHI_RT_BVH2> parent_bvh;
	std::vector<Eigen::Matrix4f> transforms;
	size_t resource_slot;
};


struct RHI_RT_HIT_GROUP_DESC2 {

	std::string group_id;
	RHI_RT_SHADER_UNIT2 closest_hit; // ClosestHit
	RHI_RT_SHADER_UNIT2 any_hit;	// AnyHit
	RHI_RT_SHADER_UNIT2 intersection; // Intersection

};

struct RHI_RT_PIPELINE_DESC2 : public RHI_PLATFORM_DESC2
{

	RHI_RT_PIPELINE_DESC2(RHI_DEVICE2& dev, RHI_PIPELINE_LAYOUT2& p_layout)
		: device(dev)
		, layout(p_layout) {
	}

	std::reference_wrapper<RHI_DEVICE2> device;
	std::reference_wrapper<RHI_PIPELINE_LAYOUT2> layout;
	std::vector<RHI_RT_HIT_GROUP_DESC2> hit_groups;
	std::vector<RHI_RT_SHADER_UNIT2> miss_shaders;
	RHI_RT_SHADER_UNIT2 ray_gen;
};









void dx12_command_queue_execute2(RHI_COMMAND_QUEUE& command_queue, bool wait_completion, fptr_command_queue_on_execute callback);
std::unique_ptr<RHI_TEXTURE_2D2> dx12_texture_2d_create2(const RHI_TEXTURE_2D_DESC2& desc);
std::unique_ptr<RHI_RENDER_PASS2> dx12_render_pass_create2(const RHI_RENDER_PASS_DESC2& desc);
std::unique_ptr<RHI_RT_PIPELINE2> dx12_rt_pipeline_create2(const RHI_RT_PIPELINE_DESC2& desc);
std::unique_ptr<RHI_PIPELINE_LAYOUT2> dx12_pipeline_layout_create2(const RHI_PIPELINE_LAYOUT_DESC2& desc);
std::unique_ptr<RHI_VERTEX_BUFFER2> dx12_vertex_buffer_create2(const RHI_VERTEX_BUFFER_DESC2& desc);
std::unique_ptr<RHI_INDEX_BUFFER2> dx12_index_buffer_create2(const RHI_INDEX_BUFFER_DESC2& desc);
void dx12_command_buffer_record2(RHI_COMMAND_BUFFER& command_buffer,
	fptr_command_buffer_on_record callback);
void dx12_render_pass_execute_rt_mode2(RHI_RENDER_PASS2& render_pass, RHI_COMMAND_BUFFER& command_buffer,
	fptr_render_pass_on_execute callback);
std::unique_ptr<RHI_CONSTANT_BUFFER2> dx12_buffers_create_constant2(const RHI_BUFFER_DESC2& desc);
std::unique_ptr<RHI_RT_BVH2> dx12_rt_bvh_create2(const RHI_RT_BVH_DESC2& desc);
std::unique_ptr<RHI_BUFFER2> dx12_rt_pipeline_create_sbt2(RHI_DEVICE2& device, RHI_RT_SBT_DESC2& desc, RHI_RT_PIPELINE2& pipeline);
void dx12_buffers_map_write2(RHI_BUFFER2& shared_buffer,
	RHI_VOID_PTR data, size_t offset,
	size_t length);
void dx12_buffers_map_close2(RHI_BUFFER2& cpu_buffer, size_t offset,
	size_t length);
RHI_VOID_PTR dx12_buffers_map_open2(RHI_BUFFER2& cpu_buffer, size_t offset,
	size_t length);
void dx12_buffers_gpu_upload2(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER2& cpu_buffer,
	RHI_BUFFER2& gpu_buffer);
std::unique_ptr<RHI_BUFFER2> dx12_buffers_create_raw2(const RHI_BUFFER_DESC2& desc);
std::unique_ptr<RHI_BUFFER2> dx12_buffers_create_2d2(const RHI_BUFFER_2D_DESC2& desc);
std::unique_ptr<RHI_BUFFER2> dx12_rt_bvh_build_geometry_instances2(const RT_GEOMETRY_INSTANCES_DESC2& desc);
void dx12_command_buffer_ray_trace2(RHI_DEVICE2& device, RHI_COMMAND_BUFFER& command_buffer,
	RHI_TEXTURE_2D2& render_target, RHI_RT_PIPELINE2& pipeline, RHI_BUFFER2& sbt);
void dx12_command_buffer_copy_texture2(RHI_COMMAND_BUFFER& command_buffer, RHI_TEXTURE_2D2& dest_texture, RHI_TEXTURE_2D2& src_texture);
std::shared_ptr<RHI_TEXTURE_2D2> dx12_swap_chain_get_surface2(RHI_SWAP_CHAIN2& swap_chain, int surface_index);
void dx12_buffers_copy_buffer2(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER2& src_buffer, RHI_BUFFER2& dest_buffer);
std::unique_ptr<D3D12_GPU_DESCRIPTOR_HANDLE> dx12_helpers_get_read_only_descriptor_heap_handle(ID3D12Device* device, ID3D12DescriptorHeap* heap, size_t slot);
std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> dx12_helpers_get_rw_descriptor_heap_handle(ID3D12Device* device, ID3D12DescriptorHeap* heap, size_t slot);
#endif