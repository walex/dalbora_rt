#include "dx12_device.hpp"
#include "dx12_factory.hpp"
#include "dx12_heap.hpp"

bool check_dx12_device_rt_support(ID3D12Device* device) {
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureData = {};
	HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureData, sizeof(featureData));
	if (FAILED(hr)) {
		return false;
	}
	return featureData.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
}

void check_dx12_device_features(ID3D12Device* i_device, const __int64 features, hlsl_shader_model shader_model) {

	bool result = true;

	auto feats = features;
	if (feats & device_features_raytracing) {
		result &= check_dx12_device_rt_support(i_device);
		feats ^= device_features_raytracing;
	}
	
	if (result == false) {
		throw std::exception("Device doesn't support requested features\n\n");
	}
	D3D12_FEATURE_DATA_SHADER_MODEL SM = {};
	SM.HighestShaderModel = D3D_HIGHEST_SHADER_MODEL;
	i_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &SM, sizeof(SM));
	if (SM.HighestShaderModel < static_cast<D3D_SHADER_MODEL>(shader_model)) {
		throw std::exception("Device doesn't support requested Shader Model\n\n");
	}
}

IDXGIAdapter1* pick_best_dx12_device_adapter(__int64 features, hlsl_shader_model shader_model) {

	IDXGIAdapter1* chosenAdapter = nullptr;
	for (UINT adapterIndex = 0;; ++adapterIndex) {
		IDXGIAdapter1* adapter = nullptr;
		HRESULT hr = dx12_factory_get()->EnumAdapters1(adapterIndex, &adapter);
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
			bool use_it = true;
			try {
				check_dx12_device_features(testDevice, features, shader_model);
			}
			catch (std::exception&) {
				
				use_it = false;
					
			}
			if (testDevice) testDevice->Release();
			if (use_it == false)
				continue;
			chosenAdapter = adapter; // keep reference (don't release)
			break;
		}
		if (adapter) adapter->Release();
	}
	return chosenAdapter;
}

RHI_DEVICE* dx12_device_create(const RHI_DEVICE_DESC* const desc) {

	ASSERT_PTR(desc);

	// Pick the best hardware adapter that supports D3D12
	IDXGIAdapter1* chosenAdapter = nullptr;
	bool check_features = true;
	if (desc->adapter_id != -1) {
		// Try to get the adapter by index
		ASSERT_SUCCESS(dx12_factory_get()->EnumAdapters1(desc->adapter_id, &chosenAdapter));
		ASSERT_PTR(chosenAdapter);
	}
	else {
		chosenAdapter = pick_best_dx12_device_adapter(desc->features, desc->shader_model);
		check_features = false;
	}

	// Create D3D12 device (request ID3D12Device). Try feature level 12_0.
	ID3D12Device* i_device = nullptr;
	ASSERT_SUCCESS(D3D12CreateDevice(chosenAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&i_device)));
	ASSERT_PTR(i_device);
	if (check_features == true) {

		try {
			check_dx12_device_features(i_device, desc->features, desc->shader_model);
		}
		catch (std::exception& ex) {
			throw ex;
		}
	}
	// Release adapter and factory references we no longer need
	DXGI_ADAPTER_DESC ad;
	chosenAdapter->GetDesc(&ad);
	chosenAdapter->Release();

	DX_DEVICE* dx_device = new DX_DEVICE;
	ASSERT_PTR(dx_device);
	dx_device->set_handle(i_device);

	return dx_device;
}

