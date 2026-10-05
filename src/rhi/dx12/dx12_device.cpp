#include "dx12_device.hpp"
#include "dx12_factory.hpp"
#include "dx12_heap.hpp"

static bool check_device_rt_support_dx12(ID3D12Device* device) {
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureData = {};
	HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureData, sizeof(featureData));
	if (FAILED(hr)) {
		return false;
	}
	return featureData.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
}

static bool check_device_mappeable_gpu_memory_dx12(ID3D12Device* device) {

	DX_DEVICE device_impl;
	device_impl.set_handle(device);
	device->AddRef();
	return dx12_memory_resource_check_type(&device_impl, buffer_memory_type_gpu_rw);
}

static void check_device_features_dx12(ID3D12Device* i_device, const __int64 features, hlsl_shader_model shader_model) {

	bool result = true;

	auto feats = features;
	if (feats & device_features_raytracing) {
		result &= check_device_rt_support_dx12(i_device);
		feats ^= device_features_raytracing;
	}
	
	if (feats & device_features_mappeable_gpu_memory) {
		result &= check_device_mappeable_gpu_memory_dx12(i_device);
		feats ^= device_features_mappeable_gpu_memory;
	}

	if (feats & device_features_dedicated_gpu) {
		D3D12_FEATURE_DATA_ARCHITECTURE archCaps = {};
		ASSERT_COM_SUCCESS(i_device->CheckFeatureSupport(
			D3D12_FEATURE_ARCHITECTURE,
			&archCaps,
			sizeof(archCaps)));
		result &= ~(archCaps.UMA);
	}

	if (result == false) {
		throw std::exception("Device doesn't support requested features\n\n");
	}

	D3D12_FEATURE_DATA_SHADER_MODEL SM = {};
	SM.HighestShaderModel = D3D_HIGHEST_SHADER_MODEL;
	ASSERT_COM_SUCCESS(i_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &SM, sizeof(SM)));

	if (SM.HighestShaderModel < static_cast<D3D_SHADER_MODEL>(shader_model)) {
		throw std::exception("Device doesn't support requested Shader Model\n\n");
	}
}

static IDXGIAdapter1* pick_best_device_adapter_dx12(__int64 features, hlsl_shader_model shader_model) {

	IDXGIAdapter1* chosenAdapter = nullptr;
	for (UINT adapterIndex = 0;; ++adapterIndex) {
		IDXGIAdapter1* adapter = nullptr;
		HRESULT hr = dx12_factory_get()->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&adapter));
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
				check_device_features_dx12(testDevice, features, shader_model);
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
		ASSERT_COM_SUCCESS(dx12_factory_get()->EnumAdapters1(desc->adapter_id, &chosenAdapter));
		ASSERT_PTR(chosenAdapter);
	}
	else {
		chosenAdapter = pick_best_device_adapter_dx12(desc->features, desc->shader_model);
		check_features = false;
	}

	// Create D3D12 device (request ID3D12Device). Try feature level 12_0.
	ID3D12Device* i_device = nullptr;
	ASSERT_COM_SUCCESS(D3D12CreateDevice(chosenAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&i_device)));
	ASSERT_PTR(i_device);
	if (check_features == true) {

		try {
			check_device_features_dx12(i_device, desc->features, desc->shader_model);
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
	dx_device->mappeable_gpu_memory = check_device_mappeable_gpu_memory_dx12(i_device);
	return dx_device;
}

