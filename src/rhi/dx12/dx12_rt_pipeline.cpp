#include "dx12_rt_pipeline.hpp"
#include "dx12_buffers.hpp"

std::unique_ptr<RHI_RT_PIPELINE> dx12_rt_pipeline_create(const RHI_RT_PIPELINE_DESC& desc) {

	ID3D12Device* i_device_0 = static_cast<ID3D12Device*>(desc.device.get());

	Microsoft::WRL::ComPtr<ID3D12Device5> i_device;
	i_device_0->QueryInterface(IID_PPV_ARGS(&i_device));

	// root signature creation
	ID3D12RootSignature* rootSignature = static_cast<DX_PIPELINE_LAYOUT&>(desc.layout.get());

	// export shaders
	// reserver for n miss, n hit, 1 ray gen
	size_t hit_group_max_elements = (desc.hit_groups.size());
	size_t ray_gen_elements = 1;
	size_t miss_max_elements = desc.miss_shaders.size();
	size_t max_exports_size = (hit_group_max_elements * 3) + miss_max_elements + ray_gen_elements;
	std::vector<D3D12_EXPORT_DESC> exports_desc(max_exports_size);
	std::vector<D3D12_HIT_GROUP_DESC> hit_groups_desc(hit_group_max_elements);
	
	std::vector<std::wstring> export_names(max_exports_size);
	std::vector<std::wstring> hit_group_names(hit_group_max_elements);
	std::vector<D3D12_DXIL_LIBRARY_DESC> libs(max_exports_size);
	std::vector<D3D12_STATE_SUBOBJECT> sub_objects;
	sub_objects.reserve(hit_group_max_elements + desc.hit_groups.size());

	size_t exports_index = 0;
	size_t hit_group_index = 0;

	// get trace ray export
	auto& unit_0 = desc.ray_gen;
	if (unit_0.blob != nullptr) {

		auto& lib = libs.at(exports_index);
		auto& name = export_names.at(exports_index);
		name = std::move(to_wstring_ascii(unit_0.name_id));
		auto& exp = exports_desc.at(exports_index++);
		exp.Name = name.c_str();
		exp.ExportToRename = nullptr;
		exp.Flags = D3D12_EXPORT_FLAG_NONE;
		IDxcBlob* i_buffer = static_cast<DX_COMPILED_SHADER_BUFFER&>(*unit_0.blob);
		lib.DXILLibrary = { i_buffer->GetBufferPointer(), i_buffer->GetBufferSize() };
		lib.NumExports = 1;
		lib.pExports = &exp;
		auto& sub_object = sub_objects.emplace_back();
		sub_object.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		sub_object.pDesc = &lib;
	}

	// get closest hit exports	
	for (size_t i = 0; i < desc.hit_groups.size(); i++) {
		
		auto& hit_group_rhi = desc.hit_groups.at(i);
		auto& hit_group_dx = hit_groups_desc.at(hit_group_index);
		auto& hit_group_name = hit_group_names.at(hit_group_index++);

		hit_group_name = std::move(to_wstring_ascii(hit_group_rhi.group_id));
		hit_group_dx.HitGroupExport = hit_group_name.c_str();
		hit_group_dx.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
		auto& unit_1 = desc.hit_groups[i].closest_hit;
		if (unit_1.blob != nullptr) {
			auto& lib = libs.at(exports_index);
			auto& name = export_names.at(exports_index);
			name = std::move(to_wstring_ascii(unit_1.name_id));
			auto& exp = exports_desc.at(exports_index++);
			exp.Name = name.c_str();
			exp.ExportToRename = nullptr;
			exp.Flags = D3D12_EXPORT_FLAG_NONE;
			IDxcBlob* i_buffer = static_cast<DX_COMPILED_SHADER_BUFFER&>(*unit_1.blob);
			lib.DXILLibrary = { i_buffer->GetBufferPointer(), i_buffer->GetBufferSize() };
			lib.NumExports = 1;
			lib.pExports = &exp;
			auto& sub_object = sub_objects.emplace_back();
			sub_object.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
			sub_object.pDesc = &lib;
			hit_group_dx.ClosestHitShaderImport = name.c_str();
		}
		auto& unit_2 = desc.hit_groups[i].any_hit;
		if (unit_2.blob != nullptr) {
			auto& lib = libs.at(exports_index);
			auto& name = export_names.at(exports_index);
			name = std::move(to_wstring_ascii(unit_2.name_id));
			auto& exp = exports_desc.at(exports_index++);
			exp.Name = name.c_str();
			exp.ExportToRename = nullptr;
			exp.Flags = D3D12_EXPORT_FLAG_NONE;
			IDxcBlob* i_buffer = static_cast<DX_COMPILED_SHADER_BUFFER&>(*unit_2.blob);
			lib.DXILLibrary = { i_buffer->GetBufferPointer(), i_buffer->GetBufferSize() };
			lib.NumExports = 1;
			lib.pExports = &exp;
			auto& sub_object = sub_objects.emplace_back();
			sub_object.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
			sub_object.pDesc = &lib;
			hit_group_dx.AnyHitShaderImport = name.c_str();
		}
		auto& unit_3 = desc.hit_groups[i].intersection;
		if (unit_3.blob != nullptr) {
			auto& lib = libs.at(exports_index);
			auto& name = export_names.at(exports_index);
			name = std::move(to_wstring_ascii(unit_3.name_id));
			auto& exp = exports_desc.at(exports_index++);
			exp.Name = name.c_str();
			exp.ExportToRename = nullptr;
			exp.Flags = D3D12_EXPORT_FLAG_NONE;
			IDxcBlob* i_buffer = static_cast<DX_COMPILED_SHADER_BUFFER&>(*unit_3.blob);
			lib.DXILLibrary = { i_buffer->GetBufferPointer(), i_buffer->GetBufferSize() };
			lib.NumExports = 1;
			lib.pExports = &exp;
			auto& sub_object = sub_objects.emplace_back();
			sub_object.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
			sub_object.pDesc = &lib;
			hit_group_dx.IntersectionShaderImport = name.c_str();
		}
		if (unit_1.blob == nullptr && unit_2.blob == nullptr && unit_3.blob == nullptr)
			continue;
		auto& sub_object = sub_objects.emplace_back();
		sub_object.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
		sub_object.pDesc = &hit_group_dx;
	}

	for (size_t i = 0; i < desc.miss_shaders.size(); i++) {
		
		auto& unit_4 = desc.miss_shaders.at(i);
		if (unit_4.blob != nullptr) {
			auto& lib = libs.at(exports_index);
			auto& name = export_names.at(exports_index);
			name = std::move(to_wstring_ascii(unit_4.name_id));
			auto& exp = exports_desc.at(exports_index++);
			exp.Name = name.c_str();
			exp.ExportToRename = nullptr;
			exp.Flags = D3D12_EXPORT_FLAG_NONE;
			IDxcBlob* i_buffer = static_cast<DX_COMPILED_SHADER_BUFFER&>(*unit_4.blob);
			lib.DXILLibrary = { i_buffer->GetBufferPointer(), i_buffer->GetBufferSize() };
			lib.NumExports = 1;
			lib.pExports = &exp;
			auto& sub_object = sub_objects.emplace_back();
			sub_object.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
			sub_object.pDesc = &lib;
		}
	}

	// [Shader Config] - Define tamaños de datos compartidos
	D3D12_RAYTRACING_SHADER_CONFIG shaderConfigDesc = {};
	// Ajusta estos valores según tus estructuras en HLSL:
	shaderConfigDesc.MaxPayloadSizeInBytes = 32;   // Ejemplo: float4 color + float profundidad
	shaderConfigDesc.MaxAttributeSizeInBytes = 8;   // Requerido por defecto para coordenadas baricéntricas (2x float)

	D3D12_STATE_SUBOBJECT shaderConfigSubobject = { D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, &shaderConfigDesc };
	sub_objects.push_back(shaderConfigSubobject);

	// [Pipeline Config] - Define límites de recursividad
	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfigDesc = {};
	pipelineConfigDesc.MaxTraceRecursionDepth = 1; // 1 para rayos primarios directos (sin rebotes)

	D3D12_STATE_SUBOBJECT pipelineConfigSubobject = { D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, &pipelineConfigDesc };
	sub_objects.push_back(pipelineConfigSubobject);

	// [Global Root Signature] - Recursos accesibles por todos los shaders del pipeline
	// pGlobalRootSignature debe ser un puntero ID3D12RootSignature ya creado previamente
	D3D12_GLOBAL_ROOT_SIGNATURE globalRSDesc = {};
	globalRSDesc.pGlobalRootSignature = rootSignature;

	D3D12_STATE_SUBOBJECT globalRSSubobject = { D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, &globalRSDesc };
	sub_objects.push_back(globalRSSubobject);

	D3D12_STATE_OBJECT_DESC pipelineDesc = {};
	pipelineDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
	pipelineDesc.NumSubobjects = static_cast<UINT>(sub_objects.size());
	pipelineDesc.pSubobjects = sub_objects.data();

	ID3D12StateObject* i_state_object = nullptr;

	HRESULT hr = i_device->CreateStateObject(
		&pipelineDesc,
		IID_PPV_ARGS(&i_state_object)
	);
	if (FAILED(hr) || !i_state_object) {
		throw std::exception("Failed to create D3D12 raytracing pipeline state object");
	}

	Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> props;

	i_state_object->QueryInterface(
		IID_PPV_ARGS(&props)
	);

	std::unique_ptr<RHI_SHADER_TABLE_ENTIRES> ste = std::make_unique<RHI_SHADER_TABLE_ENTIRES>();

	std::wstring raygen_id = std::move(to_wstring_ascii(desc.ray_gen.name_id));
	ste->emplace(desc.ray_gen.name_id, props->GetShaderIdentifier(raygen_id.c_str()));
	for (auto& ms : desc.miss_shaders) {
		std::wstring miss_id = std::move(to_wstring_ascii(ms.name_id));
		ste->emplace(ms.name_id, props->GetShaderIdentifier(miss_id.c_str()));
	}
	for (auto& hg : desc.hit_groups) {
		std::wstring group_id = std::move(to_wstring_ascii(hg.group_id));
		ste->emplace(hg.group_id, props->GetShaderIdentifier(group_id.c_str()));
	}
	return std::make_unique<DX_RT_PIPELINE>(i_state_object, desc.layout, std::move(ste));
}

std::unique_ptr<RHI_BUFFER> dx12_rt_pipeline_create_sbt(RHI_DEVICE& device, RHI_RT_SBT_DESC& desc, RHI_RT_PIPELINE& pipeline) {

	RHI_SHADER_TABLE_ENTIRES& ste = static_cast<RHI_SHADER_TABLE_ENTIRES&>(pipeline);
	
	// ============================================================
	// CONSTANTES DXR
	// ============================================================
	
	// 32 bytes
	const UINT shader_id_size =
		D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	
	// 32 bytes
	const size_t record_alignment =
		static_cast<size_t>(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
	

	// 64 bytes
	const size_t table_alignment =
		static_cast<size_t>(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

	// ============================================================
	// RECORD SIZE
	// ============================================================

	// minimal record:
	// solo shader identifier

	const size_t record_size =
		ALIGN(shader_id_size, record_alignment);

	// ============================================================
	// TABLE SIZES
	// ============================================================

	const size_t raygen_table_size =
		ALIGN(
			desc.ray_gen_ids.size() * record_size,
			table_alignment
		);

	const size_t miss_table_size =
		ALIGN(
			desc.miss_ids.size() * record_size,
			table_alignment
		);

	const size_t hitgroup_table_size =
		ALIGN(
			desc.hit_group_ids.size() * record_size,
			table_alignment
		);


	// ============================================================
	// OFFSETS
	// ============================================================

	const size_t ray_gen_offset = 0;

	const size_t miss_offset =
		ray_gen_offset + raygen_table_size;

	const size_t hit_group_offset =
		miss_offset + miss_table_size;


	// ============================================================
	// TOTAL BUFFER SIZE
	// ============================================================

	const size_t total_size =
		raygen_table_size +
		miss_table_size +
		hitgroup_table_size;

	RHI_BUFFER_DESC buffer_desc(device);
	buffer_desc.length = total_size;
	buffer_desc.memory_type = buffer_memory_type_shared_rw;
	buffer_desc.default_state = resource_state_generic_read;
	auto shared_buffer = dx12_buffers_create_raw(buffer_desc);
	ID3D12Resource* i_resource = *shared_buffer;
	i_resource->AddRef();

	// ============================================================
	// MAP
	// ============================================================

	uint8_t* mapped = static_cast<uint8_t*>(dx12_buffers_map_open(*shared_buffer, 0, total_size));

	// ============================================================
	// COPY RAYGEN RECORDS
	// ============================================================

	for (UINT i = 0; i < desc.ray_gen_ids.size(); ++i)
	{
		uint8_t* dst =
			mapped +
			ray_gen_offset +
			i * record_size;

		memcpy(
			dst,
			ste.at(desc.ray_gen_ids[i]),
			shader_id_size
		);
	}
	size_t ray_gen_size = desc.ray_gen_ids.size() * record_size;

	// ============================================================
	// COPY MISS RECORDS
	// ============================================================

	for (UINT i = 0; i < desc.miss_ids.size(); ++i)
	{
		uint8_t* dst =
			mapped +
			miss_offset +
			i * record_size;

		memcpy(
			dst,
			ste.at(desc.miss_ids[i]),
			shader_id_size
		);
	}

	size_t miss_size = desc.miss_ids.size() * record_size;

	// ============================================================
	// COPY HITGROUP RECORDS
	// ============================================================

	for (UINT i = 0; i < desc.hit_group_ids.size(); ++i)
	{
		uint8_t* dst =
			mapped +
			hit_group_offset +
			i * record_size;

		memcpy(
			dst,
			ste.at(desc.hit_group_ids[i]),
			shader_id_size
		);
	}
	size_t hit_group_size = desc.hit_group_ids.size() * record_size;

	dx12_buffers_map_close(*shared_buffer, 0, total_size);

	return std::make_unique<DX_SBT_BUFFER>(i_resource, buffer_desc.default_state, 
											buffer_desc.format, buffer_desc.length,
											ray_gen_offset, miss_offset, hit_group_offset,
											ray_gen_size, miss_size, hit_group_size,
											record_size);
}