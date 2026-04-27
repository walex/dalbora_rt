#include "dx12_device.hpp"
#include "dx12_factory.hpp"

bool dx12_device_check_rt_support(ID3D12Device* device) {
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureData = {};
	HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureData, sizeof(featureData));
	if (FAILED(hr)) {
		return false;
	}
	return featureData.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
}

bool dx12_device_check_device_features(ID3D12Device* device, __int64 features) {
	
	bool result = true;
	
	if (features & device_features_raytracing)
		result |= dx12_device_check_rt_support(device);
	
	return result;
}

IDXGIAdapter1* dx12_device_pick_best_adapter(__int64 features) {

	IDXGIAdapter1* chosenAdapter = nullptr;
	for (UINT adapterIndex = 0;; ++adapterIndex) {
		IDXGIAdapter1* adapter = nullptr;
		HRESULT hr = dx12_get_factory()->EnumAdapters1(adapterIndex, &adapter);
		if (hr == DXGI_ERROR_NOT_FOUND) {
			break;
		}
		if (FAILED(hr)) {
			if (adapter) adapter->Release();
			continue;
		}
		DXGI_ADAPTER_DESC1 adapterDesc;
		adapter->GetDesc1(&adapterDesc);
		// Skip software adapters
		if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			adapter->Release();
			continue;
		}
		// Check whether adapter supports D3D12 device creation
		ID3D12Device* testDevice = nullptr;
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&testDevice));
		if (SUCCEEDED(hr)) {
			bool device_ok = dx12_device_check_device_features(testDevice, features);
			if (testDevice) testDevice->Release();
			if (device_ok == false)
				continue;
			chosenAdapter = adapter; // keep reference (don't release)
			break;
		}
		if (adapter) adapter->Release();
	}
	return chosenAdapter;
}

std::unique_ptr<HAL_OBJECT> dx12_create_device(const HAL_DEVICE_DESC& desc) {
	
	// Pick the best hardware adapter that supports D3D12
	IDXGIAdapter1* chosenAdapter = nullptr;
	bool check_features = true;
	if (desc.adapter_id != -1) {
		// Try to get the adapter by index
		HRESULT hr = dx12_get_factory()->EnumAdapters1(desc.adapter_id, &chosenAdapter);
		if (FAILED(hr) || !chosenAdapter) {
			throw std::exception("Failed to get specified adapter");
		}
	}
	else {
		chosenAdapter = dx12_device_pick_best_adapter(desc.features);
		check_features = false;
	}

	// Create D3D12 device (request ID3D12Device5). Try feature level 12_0 then 11_0.
	ID3D12Device5* device5 = nullptr;
	HRESULT hr = D3D12CreateDevice(chosenAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device5));
	if (FAILED(hr)) {
		hr = D3D12CreateDevice(chosenAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device5));
	}
	// Release adapter and factory references we no longer need
	DXGI_ADAPTER_DESC ad;
	chosenAdapter->GetDesc(&ad);
	chosenAdapter->Release();

	printf("Using graphics device: %ls\n", ad.Description);

	if (FAILED(hr) || !device5) {
		throw std::exception("Failed to create D3D12 device");
	}

	if (check_features == true && dx12_device_check_device_features(device5, desc.features) == false)
		throw std::exception("D3D12 device does not support required features");

	return std::make_unique<HAL_OBJECT>(new DX_DEVICE_HANDLE(device5));
}

