#include "dx12_device.hpp"
#include "dx12_factory.hpp"
#include "dx12_heap.hpp"

bool dx12_device_check_rt_support(ID3D12Device* device) {
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureData = {};
	HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureData, sizeof(featureData));
	if (FAILED(hr)) {
		return false;
	}
	return featureData.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
}

void dx12_device_check_device_features(ID3D12Device* i_device, const __int64 features) {

	bool result = true;

	auto feats = features;
	if (feats & device_features_raytracing) {
		result |= dx12_device_check_rt_support(i_device);
		feats ^= device_features_raytracing;
	}
	else {
		throw std::exception("Device doesn't support RT\n\n");
	}
	D3D12_FEATURE_DATA_SHADER_MODEL SM = {};
	SM.HighestShaderModel = D3D_HIGHEST_SHADER_MODEL;
	i_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &SM, sizeof(SM));
	if (SM.HighestShaderModel < D3D_SHADER_MODEL_6_0) {
		throw std::exception("Device doesn't support Shader Model 6.9 or higher\n\n");
	}
}

IDXGIAdapter1* dx12_device_pick_best_adapter(__int64 features) {

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
				dx12_device_check_device_features(testDevice, features);
			}
			catch (std::exception& ex) {
				
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

std::unique_ptr<RHI_DEVICE> dx12_device_create(const RHI_DEVICE_DESC& desc) {

	// Pick the best hardware adapter that supports D3D12
	IDXGIAdapter1* chosenAdapter = nullptr;
	bool check_features = true;
	if (desc.adapter_id != -1) {
		// Try to get the adapter by index
		HRESULT hr = dx12_factory_get()->EnumAdapters1(desc.adapter_id, &chosenAdapter);
		if (FAILED(hr) || !chosenAdapter) {
			throw std::exception("Failed to get specified adapter");
		}
	}
	else {
		chosenAdapter = dx12_device_pick_best_adapter(desc.features);
		check_features = false;
	}

	// Create D3D12 device (request ID3D12Device). Try feature level 12_0.
	ID3D12Device* i_device = nullptr;
	HRESULT hr = D3D12CreateDevice(chosenAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&i_device));
	if (FAILED(hr)) {
		throw std::exception("Failed to create D3D12 device with feature level 12_0");
	}
	if (check_features == true) {

		try {
			dx12_device_check_device_features(i_device, desc.features);
		}
		catch (std::exception& ex) {
			throw ex;
		}
	}
	// Release adapter and factory references we no longer need
	DXGI_ADAPTER_DESC ad;
	chosenAdapter->GetDesc(&ad);
	chosenAdapter->Release();

	printf("Using graphics device: %ls\n", ad.Description);

	if (FAILED(hr) || !i_device) {
		throw std::exception("Failed to create D3D12 device");
	}
	auto dx_device = std::make_unique<DX_DEVICE>(i_device);
	if (desc.platform_desc_ptr != nullptr) {

		DX_DEVICE_DESC* device_desc = reinterpret_cast<DX_DEVICE_DESC*>(desc.platform_desc_ptr);
		if (device_desc->resources_heap_desc.enable == true) {
			printf("rs\n");
			dx_device->set_resources_heap(std::move(dx12_heap_create(i_device, device_desc->resources_heap_desc)));
		}
		if (device_desc->dsv_heap_desc.enable == true) {
			printf("dsv\n");
			dx_device->set_dsv_heap(std::move(dx12_heap_create(i_device, device_desc->dsv_heap_desc)));
		}
		if (device_desc->rtv_heap_desc.enable == true) {
			printf("rtv\n");
			dx_device->set_rtv_heap(std::move(dx12_heap_create(i_device, device_desc->rtv_heap_desc)));
		}
		if (device_desc->sampler_heap_desc.enable == true) {
			printf("sampler\n");
			dx_device->set_sampler_heap(std::move(dx12_heap_create(i_device, device_desc->sampler_heap_desc)));
		}
	}

	return dx_device;
}

