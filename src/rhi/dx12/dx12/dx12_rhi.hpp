#ifndef __dx12_rhi_hpp__
#define __dx12_rhi_hpp__

#include "rhi.hpp"
#include "dx12_api_params.hpp"
#include "dx12_helpers.hpp"

struct ID3D12Device;
struct COMReleaseDeleter { 
	void operator()(IUnknown* p) const { 
		SAFE_RELEASE2(p); 
		printf("com object %IX released!\n", (__int64)p);
	}
};

struct DX_COMMAND_BUFFER_HANDLE: public RHI_TEMPLATE_HANDLE<ID3D12GraphicsCommandList, COMReleaseDeleter> {
	
	DX_COMMAND_BUFFER_HANDLE(ID3D12GraphicsCommandList* command_buffer, ID3D12CommandAllocator* allocator)
		: RHI_TEMPLATE_HANDLE<ID3D12GraphicsCommandList, COMReleaseDeleter>(command_buffer)
		, command_allocator(std::make_unique<RHI_TEMPLATE_HANDLE<ID3D12CommandAllocator, COMReleaseDeleter>>(allocator)) {}
	operator ID3D12CommandAllocator* () {
		return *command_allocator;
	}
	std::unique_ptr<RHI_TEMPLATE_HANDLE<ID3D12CommandAllocator, COMReleaseDeleter>> command_allocator;
};

struct DX_DESCRIPTOR_POOL: public RHI_TEMPLATE_HANDLE<ID3D12DescriptorHeap, COMReleaseDeleter> {

	DX_DESCRIPTOR_POOL(ID3D12DescriptorHeap* heap, resource_type type)
		: RHI_TEMPLATE_HANDLE<ID3D12DescriptorHeap, COMReleaseDeleter>(heap) 
		, resource_type(type) {}

	resource_type resource_type;
};

using DX_DEVICE_HANDLE = RHI_TEMPLATE_HANDLE<ID3D12Device, COMReleaseDeleter>;
using DX_COMMAND_QUEUE_HANDLE = RHI_TEMPLATE_HANDLE<ID3D12CommandQueue>;
using DX_SWAP_CHAIN_HANDLE = RHI_TEMPLATE_HANDLE<IDXGISwapChain3, COMReleaseDeleter>;
using DX_RESOURCE_HANDLE = RHI_TEMPLATE_HANDLE<ID3D12Resource, COMReleaseDeleter>;
using DX_RASTER_PIPELINE_HANDLE = RHI_TEMPLATE_HANDLE<ID3D12PipelineState, COMReleaseDeleter>;
using DX_RT_PIPELINE_HANDLE = RHI_TEMPLATE_HANDLE<ID3D12StateObject, COMReleaseDeleter>;
using DX_FENCE_HANDLE = RHI_TEMPLATE_HANDLE<ID3D12Fence, COMReleaseDeleter>;
using DX_SHADER_BUFFER_HANDLE = RHI_TEMPLATE_HANDLE<IDxcBlob, COMReleaseDeleter>;
using DX_PIPELINE_LAYOUT_HANDLE = RHI_TEMPLATE_HANDLE<ID3D12RootSignature, COMReleaseDeleter>;
using DX_WINDOW_HANDLE = RHI_TEMPLATE_HANDLE<HWND>;


using DX_DEVICE_HANDLE_DEBUG = RHI_TEMPLATE_HANDLE_DEBUG<ID3D12Device, COMReleaseDeleter>;
using DX_COMMAND_QUEUE_HANDLE_DEBUG = RHI_TEMPLATE_HANDLE_DEBUG<ID3D12CommandQueue>;
using DX_SWAP_CHAIN_HANDLE_DEBUG = RHI_TEMPLATE_HANDLE_DEBUG<IDXGISwapChain3, COMReleaseDeleter>;
using DX_RESOURCE_HANDLE_DEBUG = RHI_TEMPLATE_HANDLE_DEBUG<ID3D12Resource, COMReleaseDeleter>;
using DX_RASTER_PIPELINE_HANDLE_DEBUG = RHI_TEMPLATE_HANDLE_DEBUG<ID3D12PipelineState, COMReleaseDeleter>;
using DX_RT_PIPELINE_HANDLE_DEBUG = RHI_TEMPLATE_HANDLE_DEBUG<ID3D12StateObject, COMReleaseDeleter>;
using DX_FENCE_HANDLE_DEBUG = RHI_TEMPLATE_HANDLE_DEBUG<ID3D12Fence, COMReleaseDeleter>;
using DX_SHADER_HANDLE_DEBUG = RHI_TEMPLATE_HANDLE_DEBUG<IDxcBlob, COMReleaseDeleter>;
using DX_WINDOW_HANDLE_DEBUG = RHI_TEMPLATE_HANDLE_DEBUG<HWND>;

template<typename I>
inline std::unique_ptr<I, COMReleaseDeleter> com_query_interface(RHI_OBJECT& resource) {
	I* id3dres = nullptr;
	IUnknown* iunk =  resource.handle<RHI_TEMPLATE_HANDLE<IUnknown>>();
	HRESULT hr = iunk->QueryInterface(__uuidof(I), (void**)&id3dres);
	if (FAILED(hr)) {
		throw std::exception("Failed to get interface from RHI_OBJECT");
	}
	return std::unique_ptr<I, COMReleaseDeleter>(id3dres);
}

constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE dx12_primitive_topology_type[] = {
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED, // primitive_topology_none
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, // primitive_topology_point
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, // primitive_topology_line
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, // primitive_topology_triangle
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH // primitive_topology_patch
};

constexpr D3D12_COMMAND_LIST_TYPE dx12_queue_type[] = {
	D3D12_COMMAND_LIST_TYPE_DIRECT, // queue_type_graphics
	D3D12_COMMAND_LIST_TYPE_COMPUTE, // queue_type_compute
	D3D12_COMMAND_LIST_TYPE_COPY // queue_type_copy
};

constexpr D3D12_RESOURCE_DIMENSION dx12_buffer_type[] = {

	D3D12_RESOURCE_DIMENSION_UNKNOWN,		// buffer_type_undef
	D3D12_RESOURCE_DIMENSION_BUFFER,		// buffer_type_raw,
	D3D12_RESOURCE_DIMENSION_TEXTURE1D,		// buffer_type_image_1d,
	D3D12_RESOURCE_DIMENSION_TEXTURE2D,		// buffer_type_image_2d,
	D3D12_RESOURCE_DIMENSION_TEXTURE3D		// buffer_type_image_3d
};

constexpr D3D12_RESOURCE_FLAGS buffer_resource_flags_type[] = {

	D3D12_RESOURCE_FLAG_NONE,						// buffer_resource_flags_undef,
	D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,		// buffer_resource_flags_rt,
	D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,		// buffer_resource_flags_depth,
	D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS		// buffer_resource_flags_uav,
};

constexpr DXGI_FORMAT dx12_resource_format_type[] = {
	DXGI_FORMAT_UNKNOWN, // resource_format_none
	DXGI_FORMAT_R16_UINT, // resource_format_uint16
	DXGI_FORMAT_R32_UINT, // resource_format_uint32
	DXGI_FORMAT_R8G8B8A8_UNORM, // resource_format_R8G8B8A8
	DXGI_FORMAT_R32_FLOAT, // resource_format_float
	DXGI_FORMAT_R32G32_FLOAT, // resource_format_float2
	DXGI_FORMAT_R32G32B32_FLOAT, // resource_format_float3
	DXGI_FORMAT_R32G32B32A32_FLOAT, // resource_format_float4
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT, // resource_format_d32_float_s8_uint
	DXGI_FORMAT_D24_UNORM_S8_UINT, // resource_format_d24_norm_s8_uint
	DXGI_FORMAT_D32_FLOAT, // resource_format_32_float
	DXGI_FORMAT_D16_UNORM // resource_format_d16_norm
};

constexpr D3D12_RESOURCE_STATES dx12_resource_state_type[] = {
	D3D12_RESOURCE_STATE_COMMON, // resource_state_none
	D3D12_RESOURCE_STATE_RENDER_TARGET, // resource_state_render_target
	D3D12_RESOURCE_STATE_DEPTH_READ, // resource_state_depth_read
	D3D12_RESOURCE_STATE_DEPTH_WRITE, // resource_state_depth_write
	D3D12_RESOURCE_STATE_UNORDERED_ACCESS, // resource_state_shader_write
	D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // resource_state_shader_read
	D3D12_RESOURCE_STATE_COPY_SOURCE, // resource_state_copy_src
	D3D12_RESOURCE_STATE_COPY_DEST, // resource_state_copy_dest
	D3D12_RESOURCE_STATE_PRESENT, // resource_state_present
	D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, // resource_state_rt_bvh
	D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, // resource_state_constant_buffer
	D3D12_RESOURCE_STATE_GENERIC_READ // resource_state_generic_read
};

constexpr D3D12_HEAP_TYPE dx12_heap_type[] = {
	D3D12_HEAP_TYPE_DEFAULT, // buffer_memory_type_gpu_only
	D3D12_HEAP_TYPE_UPLOAD,  // buffer_memory_type_cpu_to_gpu
	D3D12_HEAP_TYPE_READBACK // buffer_memory_type_gpu_to_cpu
};

void dx12_rhi_init();
void dx12_rhi_end();

#endif