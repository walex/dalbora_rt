#ifndef __dx12_rhi_hpp__
#define __dx12_rhi_hpp__

#include "rhi.hpp"

#define USE_DX12
#include "platform.hpp"
#undef USE_DX12

struct ID3D12Device;
struct EmptyDeleter { void operator()(IUnknown*) const {} };
struct ReleaseDeleter { void operator()(IUnknown* p) const { SAFE_RELEASE2(p); } };
template<typename T, typename U>
using rf_unique_ptr = std::unique_ptr<T, U>;

template<typename IDX12, typename DELETER>
struct DX_RHI_HANDLE : public RHI_HANDLE {
	DX_RHI_HANDLE(IDX12* ptr) : native_ptr(ptr) {}
	virtual ~DX_RHI_HANDLE() { 
		native_ptr.reset(); 
	}
	operator IDX12*() {
		return native_ptr.get();
	}
	void* get_native_handle() override {
		return static_cast<void*>(native_ptr.get());
	}
private:
	rf_unique_ptr<IDX12, DELETER> native_ptr;
};
struct DX_WINDOW_HANDLE : public RHI_HANDLE {
	DX_WINDOW_HANDLE(HWND h) : hwnd(h) {}
	operator HWND() { return hwnd; }
	void* get_native_handle() override {
		return static_cast<void*>(hwnd);
	}
private:
	HWND hwnd;
};

struct DX_RHI_RESOURCE : public RHI_RESOURCE {
	DX_RHI_RESOURCE(RHI_HANDLE* ptr) : RHI_RESOURCE(ptr) {}
	virtual void change_state(resource_state new_state) override;	
};

using DX_DEVICE_HANDLE = DX_RHI_HANDLE<ID3D12Device, ReleaseDeleter>;
using DX_COMMAND_QUEUE_HANDLE = DX_RHI_HANDLE<ID3D12CommandQueue, EmptyDeleter>;
using DX_SWAP_CHAIN_HANDLE = DX_RHI_HANDLE<IDXGISwapChain1, ReleaseDeleter>;
using DX_BUFFER_HANDLE = DX_RHI_HANDLE<ID3D12Resource, ReleaseDeleter>;
using DX_COMMAND_BUFFER_HANDLE = DX_RHI_HANDLE<ID3D12GraphicsCommandList, ReleaseDeleter>;
using DX_PIPELINE_HANDLE = DX_RHI_HANDLE<ID3D12PipelineState, ReleaseDeleter>;
using DX_FENCE_HANDLE = DX_RHI_HANDLE<ID3D12Fence, ReleaseDeleter>;

template<typename I>
inline I* dx_rhi_get_interface(RHI_OBJECT& resource) {
	I* id3dres = nullptr;
	IUnknown* iunk = static_cast<IUnknown*>(resource.get_native_impl()->get_native_handle());
	HRESULT hr = iunk->QueryInterface(__uuidof(I), (void**)&id3dres);
	if (FAILED(hr)) {
		throw std::exception("Failed to get interface from RHI_OBJECT");
	}
	return id3dres;
}

template<typename I, typename U>
inline I* dx_rhi_get_interface_from_rhi_object(RHI_OBJECT& resource) {
	return static_cast<I*>(reinterpret_cast<U&>(*(resource.get_native_impl()).get()));
}
HWND dx_rhi_get_window(RHI_HANDLE& window);


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

constexpr DXGI_FORMAT dx12_color_format_type[] = {
	DXGI_FORMAT_UNKNOWN, // color_format_none
	DXGI_FORMAT_R8G8B8A8_UNORM // color_format_R8G8B8A8
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
	D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE // resource_state_rt_bvh
};

void dx12_rhi_init();
void dx12_rhi_end();

#endif