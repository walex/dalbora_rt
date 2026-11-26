#include "dx12_factory.hpp"

static DX_FACTORY g_dx12_factory;

IDXGIFactory6* dx12_factory_get() {
	return g_dx12_factory;
}

void dx12_factory_create() {

	// Enable debug layer in debug builds
	UINT dxgiFactoryFlags = 0;
#if defined(DEBUG)
	{
		Microsoft::WRL::ComPtr<ID3D12Debug1> debug_controller;
		ASSERT_HRESULT_SUCCESS(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)));
		ASSERT_PTR(debug_controller);
		debug_controller->EnableDebugLayer();
		//debug_controller->SetEnableGPUBasedValidation(TRUE);
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif
	// Create DXGI factory
	IDXGIFactory6* i_factory = nullptr;
	ASSERT_HRESULT_SUCCESS(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&i_factory)));
	ASSERT_PTR(i_factory);
	g_dx12_factory.set_handle(i_factory);
}

void dx12_factory_destroy() {
#if defined(_DEBUG)
	Microsoft::WRL::ComPtr<IDXGIDebug1> dxgiDebug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
	{
		// DXGI_DEBUG_ALL revisa tanto DXGI como D3D11/D3D12
		// DXGI_DEBUG_RLO_DETAIL muestra el detalle de cada objeto vivo
		dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	}
#endif
}