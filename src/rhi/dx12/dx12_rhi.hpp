#ifndef __dx12_rhi_hpp__
#define __dx12_rhi_hpp__

#include "rhi.hpp"
#include "dx12_api_params.hpp"
#include "dx12_helpers.hpp"

struct DX_HEAP_DESC
{

	size_t slot_count;
	bool shader_visibility;
	resource_type resource_type;
	bool enable;
};

struct DX_DEVICE_DESC
{

	DX_HEAP_DESC rtv_heap_desc;
	DX_HEAP_DESC dsv_heap_desc;
	DX_HEAP_DESC resources_heap_desc;
	DX_HEAP_DESC sampler_heap_desc;
};

template <typename T>
struct DX_NATIVE_HANDLE : public Microsoft::WRL::ComPtr<T>
{

	DX_NATIVE_HANDLE(T *ptr) : Microsoft::WRL::ComPtr<T>(ptr) {}
	virtual ~DX_NATIVE_HANDLE() = default;
	operator T *() { return this->Get(); }
};

using DX_DEVICE_HANDLE = DX_NATIVE_HANDLE<ID3D12Device>;
using DX_COMMAND_QUEUE_HANDLE = DX_NATIVE_HANDLE<ID3D12CommandQueue>;
using DX_SWAP_CHAIN_HANDLE = DX_NATIVE_HANDLE<IDXGISwapChain3>;
using DX_RESOURCE_HANDLE = DX_NATIVE_HANDLE<ID3D12Resource>;
using DX_RASTER_PIPELINE_HANDLE = DX_NATIVE_HANDLE<ID3D12PipelineState>;
using DX_RT_PIPELINE_HANDLE = DX_NATIVE_HANDLE<ID3D12StateObject>;
using DX_FENCE_HANDLE = DX_NATIVE_HANDLE<ID3D12Fence>;
using DX_COMPILED_SHADER_BUFFER_HANDLE = DX_NATIVE_HANDLE<IDxcBlob>;
using DX_PIPELINE_LAYOUT_HANDLE = DX_NATIVE_HANDLE<ID3D12RootSignature>;
using DX_HEAP_HANDLE = DX_NATIVE_HANDLE<ID3D12DescriptorHeap>;
using DX_COMMAND_BUFFER_HANDLE = DX_NATIVE_HANDLE<ID3D12CommandList>;
using DX_COMMAND_ALLOCATOR = DX_NATIVE_HANDLE<ID3D12CommandAllocator>;

#define HANDLE_CONSTRUCTOR(name, iface, ...) \
	name(iface *ptr) : name##_HANDLE(ptr) { __VA_ARGS__ }
#define IMPLEMENT_GET_NATIVE_HANDLE(iface) \
	RHI_VOID_PTR get_native_handle() override { return static_cast<iface *>(*this); }

struct DX_HEAP : public DX_HEAP_HANDLE
{

	HANDLE_CONSTRUCTOR(DX_HEAP, ID3D12DescriptorHeap)
};

struct DX_DEVICE : public RHI_DEVICE, public DX_DEVICE_HANDLE
{

	HANDLE_CONSTRUCTOR(DX_DEVICE, ID3D12Device)
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Device)
	void set_rtv_heap(std::unique_ptr<DX_HEAP> &&heap) { m_rtv_heap = std::move(heap); }
	DX_HEAP *get_rtv_heap() { return m_rtv_heap.get(); }
	void set_dsv_heap(std::unique_ptr<DX_HEAP> &&heap) { m_dsv_heap = std::move(heap); }
	DX_HEAP *get_dsv_heap() { return m_dsv_heap.get(); }
	void set_resources_heap(std::unique_ptr<DX_HEAP> &&heap) { m_resources_heap = std::move(heap); }
	DX_HEAP *get_resources_heap() { return m_resources_heap.get(); }
	void set_sampler_heap(std::unique_ptr<DX_HEAP> &&heap) { m_sampler_heap = std::move(heap); }
	DX_HEAP *get_sampler_heap() { return m_sampler_heap.get(); }

private:
	std::unique_ptr<DX_HEAP> m_rtv_heap;
	std::unique_ptr<DX_HEAP> m_dsv_heap;
	std::unique_ptr<DX_HEAP> m_resources_heap;
	std::unique_ptr<DX_HEAP> m_sampler_heap;
};

struct DX_SWAP_CHAIN : public RHI_SWAP_CHAIN, public DX_SWAP_CHAIN_HANDLE
{

	HANDLE_CONSTRUCTOR(DX_SWAP_CHAIN, IDXGISwapChain3)
	IMPLEMENT_GET_NATIVE_HANDLE(IDXGISwapChain3)
};


struct DX_FENCE : public RHI_FENCE, public DX_FENCE_HANDLE
{

	HANDLE_CONSTRUCTOR(DX_FENCE, ID3D12Fence)
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Fence)
};

struct DX_COMPILED_SHADER_BUFFER : public RHI_COMPILED_SHADER_BUFFER, public DX_COMPILED_SHADER_BUFFER_HANDLE
{

	HANDLE_CONSTRUCTOR(DX_COMPILED_SHADER_BUFFER, IDxcBlob)
	IMPLEMENT_GET_NATIVE_HANDLE(IDxcBlob)
};

struct DX_PIPELINE_LAYOUT : public RHI_PIPELINE_LAYOUT, public DX_PIPELINE_LAYOUT_HANDLE
{

	HANDLE_CONSTRUCTOR(DX_PIPELINE_LAYOUT, ID3D12RootSignature)
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12RootSignature)
};

struct DX_RESOURCE : public DX_RESOURCE_HANDLE
{

	HANDLE_CONSTRUCTOR(DX_RESOURCE, ID3D12Resource)
};

struct DX_WINDOW : public RHI_WINDOW
{

	DX_WINDOW(HWND h, std::shared_ptr<RHI_WINDOW_CALLBACKS> callbacks) : RHI_WINDOW(static_cast<RHI_VOID_PTR>(h), callbacks) {}
};

struct DX_COMMAND_QUEUE : public RHI_COMMAND_QUEUE, public DX_COMMAND_QUEUE_HANDLE
{

	DX_COMMAND_QUEUE(ID3D12CommandQueue *i_cmd_queue, std::unique_ptr<RHI_FENCE> &&fence)
		: RHI_COMMAND_QUEUE(std::move(fence)), DX_COMMAND_QUEUE_HANDLE(i_cmd_queue) {}
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12CommandQueue)
	operator ID3D12Fence *() { return static_cast<ID3D12Fence *>(this->get_fence()); }
};

struct DX_BUFFER : public RHI_BUFFER, public DX_RESOURCE
{

	DX_BUFFER(ID3D12Resource *resource,
			  resource_state default_state,
			  resource_format resource_format, size_t length)
		: DX_RESOURCE(resource), RHI_BUFFER(default_state, resource_format, length)
	{
	}
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Resource)
};

struct DX_SBT_BUFFER : public DX_BUFFER
{

	DX_SBT_BUFFER(ID3D12Resource* resource,
		resource_state base_state,
		resource_format resource_format, size_t length,
		size_t ray_gen_offset, size_t miss_offset, size_t hit_group_offset,
		size_t ray_gen_size, size_t miss_size, size_t hit_group_size,
		size_t record_size)
		: DX_BUFFER(resource, base_state, resource_format, length)
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


struct DX_INDEX_BUFFER : public RHI_INDEX_BUFFER, public DX_RESOURCE {

	DX_INDEX_BUFFER(ID3D12Resource* resource,
		resource_state base_state,
		resource_format resource_format, size_t length, size_t stride)
		: DX_RESOURCE(resource)
		, RHI_INDEX_BUFFER(base_state, resource_format, length, stride)
	{
	}
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Resource)
};

struct DX_VERTEX_BUFFER : public RHI_VERTEX_BUFFER, public DX_RESOURCE {

	DX_VERTEX_BUFFER(ID3D12Resource* resource,
		resource_state base_state,
		resource_format resource_format, size_t length, size_t stride)
		: DX_RESOURCE(resource)
		, RHI_VERTEX_BUFFER(base_state, resource_format, length, stride)
	{
	}
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Resource)
};

struct DX_COMMAND_BUFFER : public RHI_COMMAND_BUFFER, public DX_NATIVE_HANDLE<ID3D12CommandList>
{

	DX_COMMAND_BUFFER(ID3D12CommandList *command_buffer, ID3D12CommandAllocator *allocator)
		: DX_NATIVE_HANDLE<ID3D12CommandList>(command_buffer), command_allocator(allocator)
	{
	}
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12CommandList)

	operator ID3D12CommandAllocator *()
	{
		return command_allocator.Get();
	}
	DX_COMMAND_ALLOCATOR command_allocator;
};

struct DX_RESOURCE_DESCRIPTOR : public DX_RESOURCE
{

	DX_RESOURCE_DESCRIPTOR(ID3D12Resource *resource, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle)
		: DX_RESOURCE(resource), cpu_handle(cpu_handle)
	{
	}
	operator D3D12_CPU_DESCRIPTOR_HANDLE &()
	{
		return cpu_handle;
	}

private:
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
};

struct DX_DEPTH_BUFFER : public DX_RESOURCE_DESCRIPTOR, public RHI_DEPTH_BUFFER
{

	DX_DEPTH_BUFFER(ID3D12Resource *i_depth_buffer, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
					resource_state base_state, resource_format resource_format,
					size_t width, size_t height)
		: DX_RESOURCE_DESCRIPTOR(i_depth_buffer, cpu_handle), RHI_DEPTH_BUFFER(base_state, resource_format, width, height) {}

	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Resource)
};

struct DX_CONSTANT_BUFFER : public DX_RESOURCE_DESCRIPTOR, public RHI_CONSTANT_BUFFER
{

	DX_CONSTANT_BUFFER(ID3D12Resource *i_depth_buffer, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
					   resource_state base_state, resource_format resource_format,
					   size_t length)
		: DX_RESOURCE_DESCRIPTOR(i_depth_buffer, cpu_handle), RHI_CONSTANT_BUFFER(base_state, resource_format, length)
	{
	}

	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Resource)
};

struct DX_TEXTURE_2D : public DX_RESOURCE_DESCRIPTOR, public RHI_TEXTURE_2D
{

	DX_TEXTURE_2D(ID3D12Resource *i_buffer, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
				  resource_state default_state, resource_format resource_format,
				  size_t width, size_t height, size_t physical_size,
				  std::vector<RHI_TEXTURE_MIPS>&& mips)
		: DX_RESOURCE_DESCRIPTOR(i_buffer, cpu_handle), RHI_TEXTURE_2D(default_state, resource_format, width, height, physical_size, std::move(mips))
	{
	}

	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Resource)
};

struct DX_BVH_BUFFER : public DX_RESOURCE_DESCRIPTOR, public RHI_BUFFER
{

	DX_BVH_BUFFER(ID3D12Resource* i_buffer, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
		resource_state default_state, resource_format resource_format, 
		size_t length)
		: DX_RESOURCE_DESCRIPTOR(i_buffer, cpu_handle), 
		RHI_BUFFER(default_state, resource_format, length)
	{
	}
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Resource)
};
struct DX_RENDER_PASS : public RHI_RENDER_PASS
{

	DX_RENDER_PASS(RHI_DEVICE &device, std::shared_ptr<RHI_TEXTURE_2D> rt)
		: RHI_RENDER_PASS(device, rt)
	{
	}
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12Device)
};

struct DX_RT_BVH : public RHI_RT_BVH, public DX_RESOURCE
{

	DX_RT_BVH(ID3D12Resource *blas_buffer)
		: DX_RESOURCE(blas_buffer)
	{
	}
};

struct DX_RASTER_PIPELINE : public RHI_RASTER_PIPELINE, public DX_RASTER_PIPELINE_HANDLE
{

	DX_RASTER_PIPELINE(ID3D12PipelineState* i_pipelune_state, RHI_PIPELINE_LAYOUT& layout)
		: DX_RASTER_PIPELINE_HANDLE(i_pipelune_state), RHI_RASTER_PIPELINE(layout) {
	}
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12PipelineState)


	operator DX_PIPELINE_LAYOUT& () { return static_cast<DX_PIPELINE_LAYOUT&>(static_cast<RHI_PIPELINE_LAYOUT&>(*this)); }
};

struct DX_RT_PIPELINE : public RHI_RT_PIPELINE, public DX_RT_PIPELINE_HANDLE
{
	DX_RT_PIPELINE(ID3D12StateObject* i_pipelune_state, RHI_PIPELINE_LAYOUT& layout, std::unique_ptr<RHI_SHADER_TABLE_ENTIRES> sbt)
		: DX_RT_PIPELINE_HANDLE(i_pipelune_state)
		, RHI_RT_PIPELINE(layout, std::move(sbt)) {
	}
	IMPLEMENT_GET_NATIVE_HANDLE(ID3D12StateObject)
	
	operator DX_PIPELINE_LAYOUT& () { return static_cast<DX_PIPELINE_LAYOUT&>(static_cast<RHI_PIPELINE_LAYOUT&>(*this)); }
};

struct DX_SAMPLER : public RHI_SAMPLER
{

	DX_SAMPLER( D3D12_CPU_DESCRIPTOR_HANDLE h): cpu_handle(h)
	{
	}
	operator D3D12_CPU_DESCRIPTOR_HANDLE& ()
	{
		return cpu_handle;
	}

private:
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
};

constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE dx12_primitive_topology_type[] = {
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED, // primitive_topology_none
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,	 // primitive_topology_point
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,		 // primitive_topology_line
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,	 // primitive_topology_triangle
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH		 // primitive_topology_patch
};

constexpr D3D12_COMMAND_LIST_TYPE dx12_queue_type[] = {
	D3D12_COMMAND_LIST_TYPE_DIRECT,	 // queue_type_graphics
	D3D12_COMMAND_LIST_TYPE_COMPUTE, // queue_type_compute
	D3D12_COMMAND_LIST_TYPE_COPY	 // queue_type_copy
};

constexpr D3D12_RESOURCE_DIMENSION dx12_buffer_type[] = {

	D3D12_RESOURCE_DIMENSION_UNKNOWN,	// buffer_type_undef
	D3D12_RESOURCE_DIMENSION_BUFFER,	// buffer_type_raw
	D3D12_RESOURCE_DIMENSION_TEXTURE1D, // buffer_type_image_1d
	D3D12_RESOURCE_DIMENSION_TEXTURE2D, // buffer_type_image_2d
	D3D12_RESOURCE_DIMENSION_TEXTURE3D, // buffer_type_image_3d
	D3D12_RESOURCE_DIMENSION_BUFFER,	// buffer_type_rt_bvh
	D3D12_RESOURCE_DIMENSION_BUFFER,		// buffer_type_depth_stencil
};

constexpr DXGI_FORMAT dx12_resource_format_type[] = {
	DXGI_FORMAT_UNKNOWN,			  // resource_format_none
	DXGI_FORMAT_R16_UINT,			  // resource_format_uint16
	DXGI_FORMAT_R32_UINT,			  // resource_format_uint32
	DXGI_FORMAT_R8G8B8A8_UNORM,		  // resource_format_R8G8B8A8
	DXGI_FORMAT_R32_FLOAT,			  // resource_format_float
	DXGI_FORMAT_R32G32_FLOAT,		  // resource_format_float2
	DXGI_FORMAT_R32G32B32_FLOAT,	  // resource_format_float3
	DXGI_FORMAT_R32G32B32A32_FLOAT,	  // resource_format_float4
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT, // resource_format_d32_float_s8_uint
	DXGI_FORMAT_D24_UNORM_S8_UINT,	  // resource_format_d24_norm_s8_uint
	DXGI_FORMAT_D32_FLOAT,			  // resource_format_32_float
	DXGI_FORMAT_D16_UNORM,			  // resource_format_d16_norm,
	DXGI_FORMAT_BC1_UNORM			  // resource_format_bc1_norm
};

constexpr D3D12_RESOURCE_STATES dx12_resource_state_type[] = {
	D3D12_RESOURCE_STATE_COMMON,																 // resource_state_none
	D3D12_RESOURCE_STATE_RENDER_TARGET,															 // resource_state_render_target
	D3D12_RESOURCE_STATE_DEPTH_READ,															 // resource_state_depth_read
	D3D12_RESOURCE_STATE_DEPTH_WRITE,															 // resource_state_depth_write
	D3D12_RESOURCE_STATE_UNORDERED_ACCESS,														 // resource_state_shader_write
	D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // resource_state_shader_read
	D3D12_RESOURCE_STATE_COPY_SOURCE,															 // resource_state_copy_src
	D3D12_RESOURCE_STATE_COPY_DEST,																 // resource_state_copy_dest
	D3D12_RESOURCE_STATE_PRESENT,																 // resource_state_present
	D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,										 // resource_state_rt_bvh
	D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,											 // resource_state_constant_buffer
	D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,											 // resource_state_vertex_buffer
	D3D12_RESOURCE_STATE_INDEX_BUFFER,															 // resource_index_buffer
	D3D12_RESOURCE_STATE_GENERIC_READ,															 // resource_state_generic_read
	D3D12_RESOURCE_STATE_UNORDERED_ACCESS														// resource_state_rt_render_target
};

constexpr D3D12_HEAP_TYPE dx12_heap_type[] = {
	D3D12_HEAP_TYPE_DEFAULT, // buffer_memory_type_default / buffer_memory_type_gpu_only
	D3D12_HEAP_TYPE_UPLOAD,	 // buffer_memory_type_shared_rw
	D3D12_HEAP_TYPE_READBACK // buffer_memory_type_shared_read_only
};

void dx12_rhi_init();
void dx12_rhi_end();

#endif