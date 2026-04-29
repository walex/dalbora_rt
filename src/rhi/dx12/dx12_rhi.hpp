#ifndef __dx12_rhi_hpp__
#define __dx12_rhi_hpp__

#include "rhi.hpp"
#include "dx12_helpers.hpp"

struct ID3D12Device;
struct COMReleaseDeleter { 
	void operator()(IUnknown* p) const { 
		printf("com object %IX with %d references about to release a reference ...\n", (__int64)p, p->AddRef() - 1);
		p->Release();
		SAFE_RELEASE2(p); 
		printf("com object released!\n");
	}
};

using DX_DEVICE_HANDLE = RHI_TEMPLATE_HANDLE<ID3D12Device, COMReleaseDeleter>;
using DX_COMMAND_QUEUE_HANDLE = RHI_TEMPLATE_HANDLE<ID3D12CommandQueue>;
using DX_SWAP_CHAIN_HANDLE = RHI_TEMPLATE_HANDLE<IDXGISwapChain1, COMReleaseDeleter>;
using DX_BUFFER_HANDLE = RHI_TEMPLATE_HANDLE<ID3D12Resource, COMReleaseDeleter>;
using DX_COMMAND_BUFFER_HANDLE = RHI_TEMPLATE_HANDLE<ID3D12GraphicsCommandList, COMReleaseDeleter>;
using DX_RASTER_PIPELINE_HANDLE = RHI_TEMPLATE_HANDLE<ID3D12PipelineState, COMReleaseDeleter>;
using DX_RT_PIPELINE_HANDLE = RHI_TEMPLATE_HANDLE<ID3D12StateObject, COMReleaseDeleter>;
using DX_FENCE_HANDLE = RHI_TEMPLATE_HANDLE<ID3D12Fence, COMReleaseDeleter>;
using DX_SHADER_HANDLE = RHI_TEMPLATE_HANDLE<IDxcBlob, COMReleaseDeleter>;
using DX_WINDOW_HANDLE = RHI_TEMPLATE_HANDLE<HWND>;

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

constexpr DXGI_FORMAT dx12_resource_format_type[] = {
	DXGI_FORMAT_UNKNOWN, // resource_format_none
	DXGI_FORMAT_R16_UINT, // resource_format_uint16
	DXGI_FORMAT_R32_UINT, // resource_format_uint32
	DXGI_FORMAT_R8G8B8A8_UNORM, // color_format_R8G8B8A8
	DXGI_FORMAT_R32G32B32_FLOAT // resource_format_R8G8B8A8_float
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