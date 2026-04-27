#include "dx12_factory.hpp"

static IDXGIFactory5* g_dx12_factory = nullptr;

IDXGIFactory5* dx12_get_factory() {
	return g_dx12_factory;
}

void dx12_destroy_factory() {

	if (g_dx12_factory) {

		g_dx12_factory->Release();
		g_dx12_factory = nullptr;
	}
}

void dx12_create_factory() {

	dx12_destroy_factory();

	// Enable debug layer in debug builds
	UINT dxgiFactoryFlags = 0;
#if defined(DEBUG)
	{
		ID3D12Debug* debugController = nullptr;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
			debugController->EnableDebugLayer();
			debugController->Release();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif
	// Create DXGI factory
	if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&g_dx12_factory)))) {
		throw std::exception("Failed to create DXGI factory");
	}
}