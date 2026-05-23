#include "dx12_factory.hpp"

static DX_FACTORY g_dx12_factory;

IDXGIFactory5* dx12_factory_get() {
	return g_dx12_factory;
}

void dx12_factory_create() {

	// Enable debug layer in debug builds
	UINT dxgiFactoryFlags = 0;
#if defined(DEBUG)
	{
		ID3D12Debug* debugController = nullptr;
		ASSERT_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		ASSERT_NULL(debugController);
		debugController->EnableDebugLayer();
		debugController->Release();
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif
	// Create DXGI factory
	IDXGIFactory5* i_factory = nullptr;
	ASSERT_FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&i_factory)));
	ASSERT_NULL(i_factory);
	g_dx12_factory.set_handle(i_factory);
}