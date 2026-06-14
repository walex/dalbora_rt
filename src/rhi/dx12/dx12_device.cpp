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
		result &= dx12_device_check_rt_support(i_device);
		feats ^= device_features_raytracing;
	}
	
	if (result == false) {
		throw std::exception("Device doesn't support requested features\n\n");
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
		chosenAdapter = dx12_device_pick_best_adapter(desc->features);
		check_features = false;
	}

	// Create D3D12 device (request ID3D12Device). Try feature level 12_0.
	ID3D12Device* i_device = nullptr;
	ASSERT_SUCCESS(D3D12CreateDevice(chosenAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&i_device)));
	ASSERT_PTR(i_device);
	if (check_features == true) {

		try {
			dx12_device_check_device_features(i_device, desc->features);
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

	// FixME: get from config
	DX_DEVICE_HEAP_DESC heaps_desc;
	heaps_desc.resources_heap_enable = RESOURCES_HEAP_ENABLE;
	
	heaps_desc.resources_heap_enable = RTV_HEAP_ENABLE;
	heaps_desc.resources_heap_size.resources_count = RESOURCES_HEAP_SLOT_COUNT;

	heaps_desc.rtv_heap_enable = RTV_HEAP_ENABLE;
	heaps_desc.resources_heap_size.rtv_count = RTV_HEAP_SLOT_COUNT;
	
	heaps_desc.dsv_heap_enable = DSV_HEAP_ENABLE;
	heaps_desc.resources_heap_size.dsv_count = DSV_HEAP_SLOT_COUNT;
	
	heaps_desc.sampler_heap_enable = (desc->features & device_features_enable_texture_sampling) 
		== device_features_enable_texture_sampling;
	heaps_desc.resources_heap_size.sampler_count = SAMPLER_HEAP_SLOT_COUNT;

	// FixME: get from config
	dx_device->constant_buffer_slot_start = 200;
	dx_device->rw_buffer_slot_start = 100;
	dx_device->read_only_buffer_slot_start = 0;
	dx_device->render_target_slot_start = 0;
	dx_device->depth_buffer_slot_start = 0;
	dx_device->sampler_slot_start = 0;

	if (heaps_desc.resources_heap_enable == true) {
		dx_device->resources_heap.reset(dx12_heap_create(dx_device, 
			resource_type_generic_rw_buffer, 
			&heaps_desc.resources_heap_size, true));
		if(!dx_device->resources_heap.get())
			throw std::exception("Failed to create resources heap");
	}
	
	if (heaps_desc.rtv_heap_enable == true) {
		dx_device->rtv_heap.reset(dx12_heap_create(dx_device,
			resource_type_render_target,
			&heaps_desc.resources_heap_size, false));
		if(!dx_device->rtv_heap.get())
			throw std::exception("Failed to create RTV heap");
	}

	if (heaps_desc.dsv_heap_enable == true) {
		dx_device->dsv_heap.reset(dx12_heap_create(dx_device,
			resource_type_depth_stencil_target,
			&heaps_desc.resources_heap_size, false));
		if(!dx_device->dsv_heap.get())
			throw std::exception("Failed to create DSV heap");
	}

	if (heaps_desc.sampler_heap_enable == true) {
		dx_device->sampler_heap.reset(dx12_heap_create(dx_device,
			resource_type_sampler,
			&heaps_desc.resources_heap_size, true));
		if(!dx_device->sampler_heap.get())
			throw std::exception("Failed to create sampler heap");
	}
	memcpy(&dx_device->heap_desc, &heaps_desc, sizeof(DX_DEVICE_HEAP_DESC));
	return dx_device;
}

