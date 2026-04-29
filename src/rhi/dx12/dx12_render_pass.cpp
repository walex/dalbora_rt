#include "dx12_render_pass.hpp"

std::unique_ptr<RHI_OBJECT> render_pass_create(RHI_OBJECT& image_buffer) {

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	//device->CreateRenderTargetView(backBuffer, &rtvDesc, rtvHandle);
	return nullptr;
}