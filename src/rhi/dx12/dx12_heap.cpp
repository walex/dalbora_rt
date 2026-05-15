#include "dx12_heap.hpp"
#include "dx12_helpers.hpp"

// bindless root signature
std::unique_ptr<DX_HEAP> dx12_heap_create(ID3D12Device* i_device, const DX_HEAP_DESC& desc) {

	D3D12_DESCRIPTOR_HEAP_TYPE type;
	switch (desc.resource_type) {
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

	auto com_hrap_cbv_srv_uav = dx12_helpers_create_descriptor_heap(i_device, type,
		desc.slot_count,
		(desc.shader_visibility == true)
		? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		: D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	return std::make_unique<DX_HEAP>(com_hrap_cbv_srv_uav.Detach());
}