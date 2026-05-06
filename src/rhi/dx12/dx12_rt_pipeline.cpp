#include "dx12_rt_pipeline.hpp"
#include "dx12_shaders_compiler.hpp"

std::unique_ptr<RHI_RT_PIPELINE> dx12_rt_pipeline_create(const RHI_RT_PIPELINE_DESC& desc) {

	/*
	auto device = com_query_interface<ID3D12Device5>(desc.device());

	D3D12_FEATURE_DATA_SHADER_MODEL SM;
	device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &SM, sizeof(SM));
	if (SM.HighestShaderModel < D3D_SHADER_MODEL_6_9) {
		throw std::exception("Device doesn't support Shader Model 6.9 or higher\n\n");
	}

	// TODO: Check for reording shader support when sdk available

	// root signature creation
	auto rootSignature = dx12_helpers_create_global_root_signature(device.get());

	// get DXIL library
	auto dxilLib_h = dx12_shaders_compiler_compile("neural_pbr.hlsl", "", "lib_6_9");
	IDxcBlob* dxilLib = dxilLib_h->handle<DX_SHADER_BUFFER_HANDLE>();
	
	// export shaders
	D3D12_EXPORT_DESC exports[3] = {};

	exports[0].Name = L"RayGen";
	exports[0].ExportToRename = nullptr;
	exports[0].Flags = D3D12_EXPORT_FLAG_NONE;

	exports[1].Name = L"Miss";
	exports[1].ExportToRename = nullptr;
	exports[1].Flags = D3D12_EXPORT_FLAG_NONE;

	exports[2].Name = L"ClosestHit";
	exports[2].ExportToRename = nullptr;
	exports[2].Flags = D3D12_EXPORT_FLAG_NONE;

	// DXIL Library
	D3D12_DXIL_LIBRARY_DESC dxilLibDesc = {};
	dxilLibDesc.DXILLibrary.pShaderBytecode = dxilLib->GetBufferPointer();
	dxilLibDesc.DXILLibrary.BytecodeLength = dxilLib->GetBufferSize();
	dxilLibDesc.NumExports = 3;
	dxilLibDesc.pExports = exports;

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

	ID3D12StateObject* rtStateObject = nullptr;

	HRESULT hr = device->CreateStateObject(
		&stateObjectDesc,
		IID_PPV_ARGS(&rtStateObject)
	);
	if (FAILED(hr) || !rtStateObject) {
		throw std::exception("Failed to create D3D12 raytracing pipeline state object");
	}

	return std::make_unique<RHI_OBJECT>(new DX_RT_PIPELINE_HANDLE(rtStateObject));
	*/

	return nullptr;
}