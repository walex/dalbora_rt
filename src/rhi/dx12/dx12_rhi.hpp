#ifndef __dx12_rhi_hpp__
#define __dx12_rhi_hpp__

#include "rhi.hpp"
#include "dx12_api_params.hpp"
#include "dx12_helpers.hpp"
struct DX_DEVICE_HEAP_DESC
{
	RHI_DEVICE* device;
	
	bool resources_heap_enable;
	size_t resources_heap_slot_count;

	bool rtv_heap_enable;
	size_t rtv_heap_slot_count;

	bool dsv_heap_enable;
	size_t dsv_heap_slot_count;

	bool sampler_heap_enable;
	size_t sampler_heap_slot_count;
};

template <typename T>
struct DX_HANDLE : public RHI_HANDLE
{
	virtual ~DX_HANDLE() {
#if DEBUG
		printf("Releasing handle of type %s\n", typeid(T).name());
#endif
	};
	void set_handle(RHI_VOID_PTR handle) override {

		com_ptr.Attach(static_cast<T*>(handle));
	}	

	RHI_VOID_PTR get_handle() const override { 
		return static_cast<RHI_VOID_PTR>(com_ptr.Get());
	}
	Microsoft::WRL::ComPtr<T> com_ptr;
};

enum heap_id_type {
	heap_id_type_rtv,
	heap_id_type_dsv,
	heap_id_type_resources,
	heap_id_type_sampler,
	heap_id_type_count,
};

typedef DX_HANDLE<IDXGIFactory5> DX_FACTORY;
struct DX_RESOURCE_HEAP_DESCRIPTOR
{
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_descriptor_handle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_descriptor_handle;
	size_t descriptor_size = 0;
};

struct DX_HEAP : public DX_HANDLE<ID3D12DescriptorHeap> {

	size_t count = 0;
	size_t max_count = 0;
	DX_RESOURCE_HEAP_DESCRIPTOR descriptor_handle;
};

struct DX_DEVICE : public RHI_DEVICE, public DX_HANDLE<ID3D12Device>
{
	std::unique_ptr<DX_HEAP> rtv_heap;
	std::unique_ptr<DX_HEAP> dsv_heap;
	std::unique_ptr<DX_HEAP> sampler_heap;
	std::unique_ptr<DX_HEAP> resources_heap;
};

struct DX_RESOURCE : public DX_HANDLE<ID3D12Resource> {
	std::atomic<D3D12_RESOURCE_STATES> current_state = D3D12_RESOURCE_STATE_COMMON;
};

struct DX_BUFFER : public RHI_BUFFER, public DX_RESOURCE {
};

struct DX_COMMAND_BUFFER : public RHI_COMMAND_BUFFER, public DX_HANDLE<ID3D12CommandList> {
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
};

struct DX_COMPILED_SHADER_BUFFER : 
	public RHI_COMPILED_SHADER_BUFFER, 
	public RHI_BUFFER,
	public DX_HANDLE<IDxcBlob> {

};

struct DX_EVENT {
	DX_EVENT() : handle(
		CreateEvent(nullptr, FALSE, FALSE, nullptr)) {

		ASSERT_PTR(handle);
	}
	virtual ~DX_EVENT() {
		if (handle)
			CloseHandle(handle);
	}
	operator HANDLE() const { return handle; }
private:
	HANDLE handle;
};

struct DX_COMMAND_QUEUE : public RHI_COMMAND_QUEUE, public DX_HANDLE<ID3D12CommandQueue> {
	DX_EVENT event_handle;
};

struct DX_TEXTURE_2D : public RHI_TEXTURE_2D, public DX_BUFFER {
};

struct DX_BVH_BUFFER : public RHI_BUFFER, public DX_RESOURCE {
};

struct DX_FENCE : public RHI_FENCE, public DX_HANDLE<ID3D12Fence> {
};

struct DX_VIEW : public RHI_VIEW, public DX_RESOURCE_HEAP_DESCRIPTOR {

};

struct DX_RENDER_PASS : public RHI_RENDER_PASS {
};

struct DX_RT_BVH : public RHI_RT_BVH, public DX_RESOURCE {
};

struct DX_PIPELINE_LAYOUT : public RHI_PIPELINE_LAYOUT, public DX_HANDLE<ID3D12RootSignature> {
};

struct DX_RASTER_PIPELINE : public RHI_RASTER_PIPELINE, public DX_HANDLE<ID3D12PipelineState> {
};

struct DX_RT_PIPELINE : public RHI_RT_PIPELINE, public DX_HANDLE<ID3D12StateObject> {
};

struct DX_SAMPLER : public RHI_SAMPLER {
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
};

struct DX_SBT_BUFFER : public RHI_SBT_TABLE, public DX_BUFFER
{
};

struct DX_SWAP_CHAIN : public RHI_SWAP_CHAIN, public DX_HANDLE<IDXGISwapChain3> {
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
	D3D12_RESOURCE_DIMENSION_BUFFER,	// buffer_type_depth_stencil
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

#ifdef DEBUG
	#define ASSERT_SUCCESS(expr) ASSERT_EXPR(expr == S_OK)
#else
	#define ASSERT_FAILED(expr)
#endif

#endif