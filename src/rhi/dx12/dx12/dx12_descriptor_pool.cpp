#include "dx12_descriptor_pool.hpp"
#include "dx12_helpers.hpp"

// bindless root signature
std::unique_ptr<RHI_OBJECT> dx12_descriptor_pool_create(const RHI_DESCRIPTOR_POOL_DESC& desc) {

	ID3D12Device* device = desc.device().handle<DX_DEVICE_HANDLE>();
	D3D12_DESCRIPTOR_HEAP_TYPE type;
	switch (desc.resource_type) {
		case resource_type_texture_read_only:
		case resource_type_texture_writable:
		case resource_type_buffer_read_only:
		case resource_type_buffer_writable:
		case resource_type_constant_buffer:
		case resource_type_generic_rw_buffer:
			type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			break;
		case resource_type_sampler:
			type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			break;
		case resource_type_render_target:
			type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			break;
		case resource_type_depth_stencil_target:
			type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			break;
		default:
			throw std::exception("Resource type not supported %d", (int)desc.resource_type);
	}	

	auto com_hrap_cbv_srv_uav = dx12_helpers_create_descriptor_heap(device, type, desc.slot_count, (desc.shader_visibility == true) ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	return std::make_unique<RHI_OBJECT>(new DX_DESCRIPTOR_POOL(com_hrap_cbv_srv_uav.Detach(), desc.resource_type));
}