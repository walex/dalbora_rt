#include "dx12_rt_pipeline.hpp"

std::unique_ptr<RHI_RT_PIPELINE> dx12_rt_pipeline_create(const RHI_RT_PIPELINE_DESC& desc) {

/*
	ID3D12Device* i_device_0 = static_cast<ID3D12Device*>(desc.device.get());
	ID3D12CommandList* i_command_buffer_0 = static_cast<ID3D12CommandList*>(reinterpret_cast<DX_COMMAND_BUFFER&>(desc.command_buffer.get()));

	Microsoft::WRL::ComPtr<ID3D12Device5> i_device;
	i_device_0->QueryInterface(IID_PPV_ARGS(&i_device));

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> i_command_buffer;
	i_command_buffer_0->QueryInterface(IID_PPV_ARGS(&i_command_buffer));

	// root signature creation
	auto rootSignature = dx12_helpers_create_global_root_signature(i_device_0);

	IDxcBlob* i_buffer = static_cast<IDxcBlob*>(reinterpret_cast<DX_COMPILED_SHADER_BUFFER&>(desc.shader.get()));

	// export shaders
	std::vector<D3D12_EXPORT_DESC> exports(desc.shader.entry_points.size();
	for (size_t i = 0; i < desc.shader.entry_points.size(); i++) {

		RHI_SHADER_ENTRY_POINT& ep = desc.shader.entry_points.at(i);
		exports[i].Name = ep.name.c_str(); // L"RayGen"; Miss; ClosestHit
		exports[i].ExportToRename = nullptr;
		exports[i].Flags = D3D12_EXPORT_FLAG_NONE;
	}

	// DXIL Library
	D3D12_DXIL_LIBRARY_DESC dxilLibDesc = {};
	dxilLibDesc.DXILLibrary.pShaderBytecode = i_buffer->GetBufferPointer();
	dxilLibDesc.DXILLibrary.BytecodeLength = i_buffer->GetBufferSize();
	dxilLibDesc.NumExports = exports.size();
	dxilLibDesc.pExports = exports.data();

	D3D12_STATE_SUBOBJECT dxilSubobject = {};
	dxilSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
	dxilSubobject.pDesc = &dxilLibDesc;

	// hit group
	D3D12_HIT_GROUP_DESC hitGroupDesc = {};
	hitGroupDesc.HitGroupExport = L"neural_pbr_hit_group";
	hitGroupDesc.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
	hitGroupDesc.ClosestHitShaderImport = L"ClosestHit";
	hitGroupDesc.AnyHitShaderImport = nullptr;
	hitGroupDesc.IntersectionShaderImport = nullptr;

	D3D12_STATE_SUBOBJECT hitGroupSubobject = {};
	hitGroupSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
	hitGroupSubobject.pDesc = &hitGroupDesc;

	// shader config
	D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
	shaderConfig.MaxPayloadSizeInBytes = 32;   // ej: float3 + padding
	shaderConfig.MaxAttributeSizeInBytes = 8;  // barycentrics

	D3D12_STATE_SUBOBJECT shaderConfigSubobject = {};
	shaderConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
	shaderConfigSubobject.pDesc = &shaderConfig;

	// root signature
	D3D12_GLOBAL_ROOT_SIGNATURE globalRootSig = {};
	globalRootSig.pGlobalRootSignature = rootSignature.Get(); // ya creada

	D3D12_STATE_SUBOBJECT rootSigSubobject = {};
	rootSigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
	rootSigSubobject.pDesc = &globalRootSig;

	// pipeline config
	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
	pipelineConfig.MaxTraceRecursionDepth = 1;

	D3D12_STATE_SUBOBJECT pipelineConfigSubobject = {};
	pipelineConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
	pipelineConfigSubobject.pDesc = &pipelineConfig;

	// sub-objects list
	D3D12_STATE_SUBOBJECT subobjects[5];

	subobjects[0] = dxilSubobject;
	subobjects[1] = hitGroupSubobject;
	subobjects[2] = shaderConfigSubobject;
	subobjects[3] = rootSigSubobject;
	subobjects[4] = pipelineConfigSubobject;

	// state object creation
	D3D12_STATE_OBJECT_DESC stateObjectDesc = {};
	stateObjectDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
	stateObjectDesc.NumSubobjects = _countof(subobjects);
	stateObjectDesc.pSubobjects = subobjects;

	ID3D12StateObject* i_state_object = nullptr;

	HRESULT hr = i_device->CreateStateObject(
		&stateObjectDesc,
		IID_PPV_ARGS(&i_state_object)
	);
	if (FAILED(hr) || !i_state_object) {
		throw std::exception("Failed to create D3D12 raytracing pipeline state object");
	}

	return std::make_unique<RHI_RT_PIPELINE>(new DX_RT_PIPELINE(i_state_object));
	
	*/
	return nullptr;
}