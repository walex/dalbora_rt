#include "dx12_alt.hpp"

#include "dx12_buffers.hpp"

std::unique_ptr<RHI_RENDER_PASS2> dx12_render_pass_create2(const RHI_RENDER_PASS_DESC2& desc) {

	return std::make_unique<DX_RENDER_PASS2>(
		desc.device,
		desc.render_target
	);
}

std::unique_ptr<RHI_INDEX_BUFFER2> dx12_index_buffer_create2(const RHI_INDEX_BUFFER_DESC2& desc) {

	// overwrite desc to match must have index buffer requeriments
	RHI_INDEX_BUFFER_DESC2 ib_desc_mutable = const_cast<RHI_INDEX_BUFFER_DESC2&>(desc);
	ib_desc_mutable.memory_type = buffer_memory_type_gpu_only;
	ib_desc_mutable.default_state = resource_state_none;
	ib_desc_mutable.type = buffer_type_raw;
	auto ib_impl = dx12_buffers_create_raw2(ib_desc_mutable);
	ID3D12Resource* i_ib = static_cast<DX_BUFFER2&>(*ib_impl);
	i_ib->AddRef();
	return std::make_unique<DX_INDEX_BUFFER2>(i_ib, ib_impl->get_default_state(),
		ib_impl->get_format(), ib_impl->get_length(), desc.stride);
}

std::unique_ptr<RHI_VERTEX_BUFFER2> dx12_vertex_buffer_create2(const RHI_VERTEX_BUFFER_DESC2& desc) {

	// overwrite desc to match must have vertex buffer requeriments
	RHI_VERTEX_BUFFER_DESC2 vb_desc_mutable = const_cast<RHI_VERTEX_BUFFER_DESC2&>(desc);
	vb_desc_mutable.memory_type = buffer_memory_type_gpu_only;
	vb_desc_mutable.default_state = resource_state_none;
	vb_desc_mutable.type = buffer_type_raw;
	auto vb_impl = dx12_buffers_create_raw2(vb_desc_mutable);
	ID3D12Resource* i_vb = static_cast<DX_BUFFER2&>(*vb_impl);
	i_vb->AddRef();
	return std::make_unique<DX_VERTEX_BUFFER2>(i_vb, vb_impl->get_default_state(),
		vb_impl->get_format(), vb_impl->get_length(), desc.stride);
}



std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> dx12_helpers_get_rw_descriptor_heap_handle(ID3D12Device* device, ID3D12DescriptorHeap* heap, size_t slot) {

	D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
	if (slot + 1 > (int)desc.NumDescriptors) {
		throw std::exception("Max descriptors reached for type %d", desc.Type);
	}
	UINT rtvDescriptorSize =
		device->GetDescriptorHandleIncrementSize(desc.Type);
	auto h = heap->GetCPUDescriptorHandleForHeapStart();
	h.ptr += (slot * rtvDescriptorSize);
	return std::make_unique<D3D12_CPU_DESCRIPTOR_HANDLE>(h);
}



std::unique_ptr<RHI_TEXTURE_2D2> dx12_texture_2d_create2(const RHI_TEXTURE_2D_DESC2& desc)
{
	DX_DEVICE2& device_impl = reinterpret_cast<DX_DEVICE2&>(desc.device.get());
	ID3D12Device* i_device = device_impl;
	DX_HEAP2* heap_impl = device_impl.get_resources_heap();
	if (heap_impl == nullptr)
	{
		throw std::exception("NO heap found for dsv.");
	}

	std::unique_ptr<RHI_BUFFER2> buffer = dx12_buffers_create_2d2(desc);
	ID3D12Resource* i_texture = static_cast<DX_BUFFER2&>(*buffer);
	i_texture->AddRef();


	size_t heap_slot = desc.resource_slot;

	ID3D12DescriptorHeap* i_heap = *heap_impl;
	std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> srv_handle = dx12_helpers_get_rw_descriptor_heap_handle(i_device, i_heap, heap_slot);

	if (desc.default_state == resource_state_rt_render_target) {
		D3D12_UNORDERED_ACCESS_VIEW_DESC uav = {};

		uav.Format =
			dx12_resource_format_type[desc.format];

		uav.ViewDimension =
			D3D12_UAV_DIMENSION_TEXTURE2D;

		i_device->CreateUnorderedAccessView(
			i_texture,
			nullptr,
			&uav,
			*srv_handle
		);
	}
	else {
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = dx12_resource_format_type[desc.format];
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping =
			D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = static_cast<UINT>(desc.mips);
		srvDesc.Texture1D.MostDetailedMip = 0;
		i_device->CreateShaderResourceView(
			i_texture,
			&srvDesc,
			*srv_handle);
	}

	const UINT mip_count = static_cast<UINT>(desc.mips);
	std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>
		layouts(mip_count);
	std::vector<UINT> num_rows(mip_count);
	std::vector<UINT64> row_sizes(mip_count);
	UINT64 totalUploadSize = 0;

	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = static_cast<UINT>(desc.width);
	texDesc.Height = static_cast<UINT>(desc.height);
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = mip_count;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	texDesc.Format = dx12_resource_format_type[(int)desc.format];

	i_device->GetCopyableFootprints(
		&texDesc,
		0,
		mip_count,
		0,
		layouts.data(),
		num_rows.data(),
		row_sizes.data(),
		&totalUploadSize);

	std::vector<RHI_TEXTURE_MIPS> mips(mip_count);
	for (UINT i = 0; i < mip_count; i++) {
		mips[i].offset = static_cast<size_t>(layouts[i].Offset);
		mips[i].num_rows = static_cast<size_t>(num_rows[i]);
		mips[i].pitch = static_cast<size_t>(layouts[i].Footprint.RowPitch);
		mips[i].width = static_cast<size_t>(layouts[i].Footprint.Width);
		mips[i].height = static_cast<size_t>(layouts[i].Footprint.Height);
		mips[i].depth = static_cast<size_t>(layouts[i].Footprint.Depth);
		mips[i].format = dx12_helpers_resource_format_from_dxgi_format(layouts[i].Footprint.Format);
	}

	return std::make_unique<DX_TEXTURE_2D2>(i_texture, *srv_handle,
		desc.default_state, desc.format,
		static_cast<size_t>(desc.width), static_cast<size_t>(desc.height),
		static_cast<size_t>(totalUploadSize), std::move(mips));
}

std::unique_ptr<RHI_RT_PIPELINE2> dx12_rt_pipeline_create2(const RHI_RT_PIPELINE_DESC2& desc) {

	ID3D12Device* i_device_0 = static_cast<ID3D12Device*>(desc.device.get());

	Microsoft::WRL::ComPtr<ID3D12Device5> i_device;
	i_device_0->QueryInterface(IID_PPV_ARGS(&i_device));

	// root signature creation
	ID3D12RootSignature* rootSignature = static_cast<DX_PIPELINE_LAYOUT2&>(desc.layout.get());

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

	std::unique_ptr<RHI_SHADER_TABLE_ENTIRES2> ste = std::make_unique<RHI_SHADER_TABLE_ENTIRES2>();

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
	return std::make_unique<DX_RT_PIPELINE2>(i_state_object, desc.layout, std::move(ste));
}

std::unique_ptr<RHI_PIPELINE_LAYOUT2> dx12_pipeline_layout_create2(const RHI_PIPELINE_LAYOUT_DESC2& desc) {


	std::vector<D3D12_DESCRIPTOR_RANGE1> descriptor_ranges;
	std::vector<D3D12_DESCRIPTOR_RANGE1> descriptor_ranges_sampler;

	for (size_t i = 0; i < desc.descriptors.size(); i++) {

		auto& descriptor = desc.descriptors.at(i);
		D3D12_DESCRIPTOR_RANGE_TYPE range_type;

		switch (descriptor.resource_type) {
		case resource_type_shader:
			range_type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			break;
		case resource_type_generic_rw_buffer:
			range_type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			break;
		case resource_type_constant_buffer:
			range_type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			break;
		case resource_type_sampler:
			range_type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			break;
		default:
			throw std::exception("Resource type not supported %d", (int)descriptor.resource_type);
		}

		D3D12_DESCRIPTOR_RANGE1* range = (range_type == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
			? &descriptor_ranges_sampler.emplace_back()
			: &descriptor_ranges.emplace_back();
		range->RangeType = range_type;
		range->BaseShaderRegister = descriptor.pool_range_start;
		range->NumDescriptors = descriptor.pool_range_count;
		range->RegisterSpace = 0;
		range->Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
		range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

	std::vector<D3D12_ROOT_PARAMETER1> root_params;
	if (descriptor_ranges.size() > 0) {

		D3D12_ROOT_PARAMETER1& root_parameter = root_params.emplace_back();
		root_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		root_parameter.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(descriptor_ranges.size());
		root_parameter.DescriptorTable.pDescriptorRanges = descriptor_ranges.data();
		root_parameter.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(desc.shader_type);
	}
	if (descriptor_ranges_sampler.size() > 0) {

		D3D12_ROOT_PARAMETER1& root_parameter = root_params.emplace_back();
		root_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		root_parameter.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(descriptor_ranges_sampler.size());
		root_parameter.DescriptorTable.pDescriptorRanges = descriptor_ranges_sampler.data();
		root_parameter.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(desc.shader_type);
	}

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC  rootDesc = {};
	rootDesc.Version =
		D3D_ROOT_SIGNATURE_VERSION_1_1;
	rootDesc.Desc_1_1.NumParameters = static_cast<UINT>(root_params.size());
	rootDesc.Desc_1_1.pParameters = root_params.data();
	rootDesc.Desc_1_1.NumStaticSamplers = 0;
	rootDesc.Desc_1_1.pStaticSamplers = nullptr;
	rootDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	Microsoft::WRL::ComPtr<ID3DBlob> error;
	Microsoft::WRL::ComPtr<ID3DBlob> signature;

	HRESULT hr = D3D12SerializeVersionedRootSignature(
		&rootDesc,
		&signature,
		&error
	);
	if (FAILED(hr)) {
		throw std::exception("Failed serializing root signature");
	}
	ID3D12RootSignature* i_root_signature = nullptr;
	ID3D12Device* i_device = desc.device.get();
	hr = i_device->CreateRootSignature(
		0,
		signature->GetBufferPointer(),
		signature->GetBufferSize(),
		IID_PPV_ARGS(&i_root_signature)
	);
	if (FAILED(hr)) {
		throw std::exception("Failed creating root signature");
	}
	return std::make_unique<DX_PIPELINE_LAYOUT2>(i_root_signature);
}

void dx12_command_queue_wait2(RHI_COMMAND_QUEUE& command_queue) {

	HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (!eventHandle) {
		throw std::exception("Failed to create event for command queue wait");
	}
	ID3D12CommandQueue* i_cmd_queue = static_cast<DX_COMMAND_QUEUE&>(command_queue);
	ID3D12Fence* i_fence = *static_cast<DX_FENCE*>(static_cast<DX_COMMAND_QUEUE&>(command_queue).fence.get());
	auto fc = command_queue.fence_counter++;
	i_cmd_queue->Signal(i_fence, fc);
	if (i_fence->GetCompletedValue() < fc) {
		// Wait for the fence to be signaled
		i_fence->SetEventOnCompletion(fc, eventHandle);
		WaitForSingleObjectEx(eventHandle, INFINITE, FALSE);
	}
	CloseHandle(eventHandle);

}

void dx12_command_queue_execute2(RHI_COMMAND_QUEUE& command_queue, bool wait_completion, fptr_command_queue_on_execute callback) {

	std::vector<RHI_COMMAND_BUFFER*> command_buffer_list;
	ID3D12CommandQueue* i_cmd_queue = static_cast<DX_COMMAND_QUEUE&>(command_queue);
	callback(static_cast<RHI_VOID_PTR>(i_cmd_queue), &command_buffer_list);
	size_t list_size = command_buffer_list.size();
	if (list_size > 0) {
		std::vector<ID3D12CommandList*> native_list(list_size);
		for (int i = 0; i < list_size; i++) {
			ID3D12GraphicsCommandList* i_cmd_list = *static_cast<DX_COMMAND_BUFFER*>(command_buffer_list[i]);
			native_list[i] = i_cmd_list;
		}
		i_cmd_queue->ExecuteCommandLists(1, native_list.data());
		if (wait_completion == true) {
			dx12_command_queue_wait2(command_queue);
		}
	}
}

void dx12_command_buffer_record2(RHI_COMMAND_BUFFER& command_buffer,
	fptr_command_buffer_on_record callback) {

	auto& cmd_buffer_impl = reinterpret_cast<DX_COMMAND_BUFFER&>(command_buffer);
	ID3D12GraphicsCommandList* i_cmd_list = cmd_buffer_impl;
	ID3D12CommandAllocator* i_cmd_alloc = cmd_buffer_impl.allocator.Get();
	i_cmd_alloc->Reset();
	i_cmd_list->Reset(i_cmd_alloc, nullptr);

	callback(static_cast<RHI_VOID_PTR>(i_cmd_list));

	i_cmd_list->Close();
}

void dx12_render_pass_execute_rt_mode2(RHI_RENDER_PASS2& render_pass, RHI_COMMAND_BUFFER& command_buffer,
	fptr_render_pass_on_execute callback) {

	RHI_DEPTH_BUFFER2* depth_buffer_impl = render_pass;
	RHI_GRAPHICS_PIPELINE2* pipeline_impl = render_pass;
	RHI_TEXTURE_2D2& render_target_impl = render_pass;
	DX_DEVICE2& device_impl = static_cast<DX_DEVICE2&>(static_cast<RHI_DEVICE2&>(render_pass));
	ID3D12GraphicsCommandList* i_command_buffer = static_cast<DX_COMMAND_BUFFER&>(command_buffer);
	DX_RT_PIPELINE2& dx_pipeline_impl = static_cast<DX_RT_PIPELINE2&>(*pipeline_impl);
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList5> i_command_buffer_5;

	resource_state old_state = render_target_impl.get_current_state();
	D3D12_RESOURCE_BARRIER barrier = {};
	if (old_state != resource_state_rt_render_target) {
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = render_target_impl;
		barrier.Transition.StateBefore = dx12_resource_state_type[old_state];
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		i_command_buffer->ResourceBarrier(1, &barrier);
		render_target_impl.set_current_state(resource_state_rt_render_target);
	}
	i_command_buffer->QueryInterface(IID_PPV_ARGS(&i_command_buffer_5));
	ID3D12DescriptorHeap* heaps[] =
	{
		device_impl.get_resources_heap()->Get()
	}; 
	
	ID3D12Device* dev = device_impl.Get();
	DX_HEAP2* h2 = device_impl.get_resources_heap();
	ID3D12DescriptorHeap* heap = h2->Get();

	auto handle = dx12_helpers_get_read_only_descriptor_heap_handle(dev, heap , 0);
	i_command_buffer_5->SetDescriptorHeaps(_countof(heaps), heaps);
	i_command_buffer_5->SetComputeRootSignature(static_cast<DX_PIPELINE_LAYOUT2&>(static_cast<DX_RT_PIPELINE2&>(*pipeline_impl)));
	i_command_buffer_5->SetComputeRootDescriptorTable(0, *handle.get());
	i_command_buffer_5->SetPipelineState1(static_cast<DX_RT_PIPELINE2&>(*pipeline_impl));

	if (callback)
		callback();

	if (old_state != resource_state_rt_render_target) {
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.StateAfter = dx12_resource_state_type[old_state];
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		i_command_buffer->ResourceBarrier(1, &barrier);
		render_target_impl.set_current_state(old_state);
	}
}

std::unique_ptr<RHI_CONSTANT_BUFFER2> dx12_buffers_create_constant2(const RHI_BUFFER_DESC2& desc)
{
	DX_DEVICE2& device_impl = reinterpret_cast<DX_DEVICE2&>(desc.device.get());
	DX_HEAP2* heap_impl = device_impl.get_resources_heap();
	if (heap_impl == nullptr)
	{
		throw std::exception("NO heap found for dsv.");
	}
	auto buffer_impl = dx12_buffers_create_raw2(desc);

	ID3D12Device* i_device = device_impl;
	ID3D12DescriptorHeap* i_heap = *heap_impl;

	// set heap offset
	size_t heap_slot = desc.resource_slot;
	// keep resource alive
	ID3D12Resource* i_resource = *buffer_impl;
	i_resource->AddRef();

	// create view
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
	cbv_desc.BufferLocation = i_resource->GetGPUVirtualAddress();
	cbv_desc.SizeInBytes = static_cast<UINT>(desc.length); // MUST BE ALIGNED
	std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> cvb_handle = dx12_helpers_get_rw_descriptor_heap_handle(i_device, i_heap, heap_slot);
	i_device->CreateConstantBufferView(&cbv_desc, *cvb_handle);
	return std::make_unique<DX_CONSTANT_BUFFER2>(i_resource, *cvb_handle,
		desc.default_state, desc.format,
		desc.length);
}

std::unique_ptr<RHI_RT_BVH2> dx12_rt_bvh_create2(const RHI_RT_BVH_DESC2& desc)
{

	ID3D12Device* i_device_0 = static_cast<DX_DEVICE2&>(desc.device.get());
	ID3D12CommandList* i_command_buffer_0 = static_cast<DX_COMMAND_BUFFER&>(desc.command_buffer.get());
	DX_VERTEX_BUFFER2& vb_impl = static_cast<DX_VERTEX_BUFFER2&>(desc.vertex_buffer.get());
	DX_INDEX_BUFFER2* ib_impl = static_cast<DX_INDEX_BUFFER2*>(desc.index_buffer);
	ID3D12Resource* i_vb = vb_impl;

	Microsoft::WRL::ComPtr<ID3D12Device5> i_device;
	i_device_0->QueryInterface(IID_PPV_ARGS(&i_device));

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> i_command_buffer;
	i_command_buffer_0->QueryInterface(IID_PPV_ARGS(&i_command_buffer));

	// create blas
	D3D12_RAYTRACING_GEOMETRY_DESC geomDesc = {};
	geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	geomDesc.Triangles.VertexBuffer.StartAddress =
		i_vb->GetGPUVirtualAddress();
	geomDesc.Triangles.VertexBuffer.StrideInBytes = vb_impl.get_stride();
	geomDesc.Triangles.VertexCount = (UINT)(vb_impl.get_length() / vb_impl.get_stride());
	geomDesc.Triangles.VertexFormat = dx12_resource_format_type[vb_impl.get_format()];

	if (ib_impl)
	{

		ID3D12Resource* i_ib = *ib_impl;
		geomDesc.Triangles.IndexBuffer =
			i_ib->GetGPUVirtualAddress();
		geomDesc.Triangles.IndexCount = (UINT)(ib_impl->get_length() / ib_impl->get_stride());
		geomDesc.Triangles.IndexFormat = dx12_resource_format_type[ib_impl->get_format()];
	}

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.NumDescs = 1;
	inputs.pGeometryDescs = &geomDesc;

	// pre build info
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO blasInfo = {};
	i_device->GetRaytracingAccelerationStructurePrebuildInfo(
		&inputs,
		&blasInfo);

	RHI_BUFFER_DESC2 buffer_desc(desc.device);
	buffer_desc.default_state = resource_state_rt_bvh;
	buffer_desc.type = buffer_type_rt_bvh;
	buffer_desc.format = desc.vertex_buffer.get().get_format();
	buffer_desc.memory_type = buffer_memory_type_gpu_only;
	buffer_desc.length = blasInfo.ResultDataMaxSizeInBytes;
	auto blas_buffer_impl = dx12_buffers_create_raw2(buffer_desc);
	buffer_desc.length = blasInfo.ScratchDataSizeInBytes;
	buffer_desc.type = buffer_type_rt_bvh;
	buffer_desc.default_state = resource_state_none;
	auto scratch_buffer_impl = dx12_buffers_create_raw2(buffer_desc);

	ID3D12Resource* i_blas_buffer = *blas_buffer_impl;
	ID3D12Resource* i_scratch_buffer = *scratch_buffer_impl;

	// build
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc = {};
	build_desc.Inputs = inputs;
	build_desc.DestAccelerationStructureData = i_blas_buffer->GetGPUVirtualAddress();
	build_desc.ScratchAccelerationStructureData = i_scratch_buffer->GetGPUVirtualAddress();
	i_command_buffer->BuildRaytracingAccelerationStructure(&build_desc, 0, nullptr);

	// UAV barrier BLAS
	D3D12_RESOURCE_BARRIER blas_barrier = {};
	blas_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	blas_barrier.UAV.pResource = i_blas_buffer;

	i_command_buffer->ResourceBarrier(1, &blas_barrier);

	i_blas_buffer->AddRef();
	blas_buffer_impl->set_current_state(resource_state_rt_bvh);

	return std::make_unique<DX_RT_BVH2>(i_blas_buffer);
}

std::unique_ptr<RHI_BUFFER2> dx12_rt_pipeline_create_sbt2(RHI_DEVICE2& device, RHI_RT_SBT_DESC2& desc, RHI_RT_PIPELINE2& pipeline) {

	RHI_SHADER_TABLE_ENTIRES2& ste = static_cast<RHI_SHADER_TABLE_ENTIRES2&>(pipeline);

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

	RHI_BUFFER_DESC2 buffer_desc(device);
	buffer_desc.length = total_size;
	buffer_desc.memory_type = buffer_memory_type_shared_rw;
	buffer_desc.default_state = resource_state_generic_read;
	auto shared_buffer = dx12_buffers_create_raw2(buffer_desc);
	ID3D12Resource* i_resource = *shared_buffer;
	i_resource->AddRef();

	// ============================================================
	// MAP
	// ============================================================

	uint8_t* mapped = static_cast<uint8_t*>(dx12_buffers_map_open2(*shared_buffer, 0, total_size));

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

	dx12_buffers_map_close2(*shared_buffer, 0, total_size);

	return std::make_unique<DX_SBT_BUFFER2>(i_resource, buffer_desc.default_state,
		buffer_desc.format, buffer_desc.length,
		ray_gen_offset, miss_offset, hit_group_offset,
		ray_gen_size, miss_size, hit_group_size,
		record_size);
}

RHI_VOID_PTR dx12_buffers_map_open2(RHI_BUFFER2& cpu_buffer, size_t offset,
	size_t length)
{

	ID3D12Resource* i_shared_buffer = cpu_buffer;
	D3D12_RANGE range{
		.Begin = offset,
		.End = length };

	RHI_VOID_PTR mapped;
	if (FAILED(i_shared_buffer->Map(0, &range, &mapped)))
	{
		throw std::exception("Error cannot map resource");
	}
	return mapped;
}

void dx12_buffers_map_close2(RHI_BUFFER2& cpu_buffer, size_t offset,
	size_t length)
{

	ID3D12Resource* i_shared_buffer = cpu_buffer;
	D3D12_RANGE range{
		.Begin = offset,
		.End = length };
	i_shared_buffer->Unmap(0, &range);
}

void dx12_buffers_map_write2(RHI_BUFFER2& shared_buffer,
	RHI_VOID_PTR data, size_t offset,
	size_t length)
{
	ID3D12Resource* i_shared_buffer = shared_buffer;
	D3D12_RANGE range{
		.Begin = offset,
		.End = length };
	void* mapped;
	if (FAILED(i_shared_buffer->Map(0, &range, &mapped)))
	{
		throw std::exception("Error cannot map resource");
	}
	memcpy(mapped, data, length);
	i_shared_buffer->Unmap(0, &range);
}

void dx12_buffers_gpu_upload2(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER2& cpu_buffer,
	RHI_BUFFER2& gpu_buffer)
{

	ID3D12GraphicsCommandList* i_cmd_list = static_cast<DX_COMMAND_BUFFER&>(command_buffer);
	ID3D12Resource* i_dest_buffer = gpu_buffer;

	auto old_state = gpu_buffer.get_current_state();
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = i_dest_buffer;
	barrier.Transition.StateBefore = dx12_resource_state_type[old_state];
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	i_cmd_list->ResourceBarrier(1, &barrier);
	gpu_buffer.set_current_state(resource_state_copy_dest);
	dx12_buffers_copy_buffer2(command_buffer, cpu_buffer,
		gpu_buffer);
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = dx12_resource_state_type[old_state];
	i_cmd_list->ResourceBarrier(1, &barrier);
	gpu_buffer.set_current_state(old_state);
}

std::unique_ptr<RHI_BUFFER2> dx12_buffers_create_2d2(const RHI_BUFFER_2D_DESC2& desc)
{

	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

	std::unique_ptr<D3D12_CLEAR_VALUE> clear_value;
	D3D12_RESOURCE_STATES resource_initial_state;
	buffer_type buffer_type = desc.type;
	if (buffer_type == buffer_type_depth_stencil)
	{

		flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		clear_value = std::make_unique<D3D12_CLEAR_VALUE>();
		clear_value->Format = dx12_resource_format_type[(int)desc.format];
		clear_value->DepthStencil.Depth = 1.0f;
		clear_value->DepthStencil.Stencil = 0;
		buffer_type = buffer_type_image_2d;
	}
	else if (buffer_type == buffer_type_rt_bvh
		|| desc.default_state == resource_state_rt_render_target) {
		flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	if (desc.default_state == resource_state_rt_render_target
		|| desc.default_state == resource_state_raster_render_target) {

		flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}
	resource_initial_state = dx12_resource_state_type[desc.default_state];
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = dx12_heap_type[desc.memory_type];
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = dx12_buffer_type[(int)buffer_type];
	bufferDesc.Alignment = 0;
	bufferDesc.Width = static_cast<UINT>(desc.width);
	bufferDesc.Height = static_cast<UINT>(desc.height);
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = static_cast<UINT>(desc.mips);
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	if (bufferDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	else
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	bufferDesc.Flags = flags;
	bufferDesc.Format = (bufferDesc.Dimension > D3D12_RESOURCE_DIMENSION_BUFFER)
		? dx12_resource_format_type[(int)desc.format]
		: DXGI_FORMAT_UNKNOWN;
	ID3D12Resource* i_resource = nullptr;
	ID3D12Device* i_device = desc.device.get();
	HRESULT hr = i_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		resource_initial_state,
		clear_value.get(),
		IID_PPV_ARGS(&i_resource));
	if (FAILED(hr) || !i_resource)
	{
		throw std::exception("Failed to create D3D12 resource");
	}
	return std::make_unique<DX_BUFFER2>(i_resource, desc.default_state, desc.format, desc.length);
}

std::unique_ptr<RHI_BUFFER2> dx12_buffers_create_raw2(const RHI_BUFFER_DESC2& desc)
{
	RHI_BUFFER_2D_DESC2 desc_2d(desc.device);
	desc_2d.length = desc.length;
	desc_2d.mips = desc.mips;
	desc_2d.default_state = desc.default_state;
	desc_2d.memory_type = desc.memory_type;
	desc_2d.format = desc.format;
	desc_2d.type = desc.type;
	desc_2d.width = desc.length;
	desc_2d.height = 1;
	return dx12_buffers_create_2d2(desc_2d);
}

std::unique_ptr<RHI_BUFFER2> dx12_rt_bvh_build_geometry_instances2(const RT_GEOMETRY_INSTANCES_DESC2& desc)
{

	ID3D12Resource* i_blas_buffer = static_cast<DX_RT_BVH2&>(desc.parent_bvh.get());
	DX_DEVICE2& device_impl = reinterpret_cast<DX_DEVICE2&>(desc.device.get());

	// TLAS

	RHI_BUFFER_DESC2 inputs_buffer_desc(device_impl);
	inputs_buffer_desc.length = desc.transforms.size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);
	inputs_buffer_desc.memory_type = buffer_memory_type_shared_rw;
	inputs_buffer_desc.default_state = resource_state_generic_read;
	auto tlas_inputs_buffer_impl = dx12_buffers_create_raw2(inputs_buffer_desc);
	ID3D12Resource* i_tlas_inputs_buffer = static_cast<DX_BUFFER2&>(*tlas_inputs_buffer_impl);

	D3D12_RAYTRACING_INSTANCE_DESC* instances = nullptr;
	D3D12_RANGE readRange(0, 0);

	i_tlas_inputs_buffer->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&instances));

	UINT iid = 0;
	for (size_t i = 0; i < desc.transforms.size(); i++)
	{

		D3D12_RAYTRACING_INSTANCE_DESC& instance = instances[i];
		instance.InstanceID = iid++;
		instance.InstanceMask = 0xFF;
		instance.AccelerationStructure = i_blas_buffer->GetGPUVirtualAddress();

		auto& mat = desc.transforms[i];

		// fila 0
		instance.Transform[0][0] = mat(0, 0);
		instance.Transform[0][1] = mat(0, 1);
		instance.Transform[0][2] = mat(0, 2);
		instance.Transform[0][3] = mat(0, 3);

		// fila 1
		instance.Transform[1][0] = mat(1, 0);
		instance.Transform[1][1] = mat(1, 1);
		instance.Transform[1][2] = mat(1, 2);
		instance.Transform[1][3] = mat(1, 3);

		// fila 2
		instance.Transform[2][0] = mat(2, 0);
		instance.Transform[2][1] = mat(2, 1);
		instance.Transform[2][2] = mat(2, 2);
		instance.Transform[2][3] = mat(2, 3);
	}

	i_tlas_inputs_buffer->Unmap(0, nullptr);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlas_inputs = {};
	tlas_inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	tlas_inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	tlas_inputs.NumDescs = 1;
	tlas_inputs.InstanceDescs = i_tlas_inputs_buffer->GetGPUVirtualAddress();

	Microsoft::WRL::ComPtr<ID3D12Device5> i_device;
	static_cast<ID3D12Device*>(device_impl)->QueryInterface(IID_PPV_ARGS(&i_device));
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO tlas_info = {};
	i_device->GetRaytracingAccelerationStructurePrebuildInfo(&tlas_inputs, &tlas_info);

	RHI_BUFFER_DESC2 buffer_desc(desc.device);
	buffer_desc.default_state = resource_state_rt_bvh;
	buffer_desc.type = buffer_type_rt_bvh;
	// buffer_desc.format = desc.vertex_buffer.get().get_format();
	buffer_desc.memory_type = buffer_memory_type_gpu_only;
	buffer_desc.length = tlas_info.ResultDataMaxSizeInBytes;
	auto tlas_buffer_impl = dx12_buffers_create_raw2(buffer_desc);
	buffer_desc.length = tlas_info.ScratchDataSizeInBytes;
	buffer_desc.type = buffer_type_rt_bvh;
	buffer_desc.default_state = resource_state_none;
	auto scratch_buffer_impl = dx12_buffers_create_raw2(buffer_desc);

	ID3D12Resource* i_tlas_buffer = *tlas_buffer_impl;
	ID3D12Resource* i_scratch_buffer = *scratch_buffer_impl;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasBuild = {};
	tlasBuild.Inputs = tlas_inputs;
	tlasBuild.DestAccelerationStructureData = i_tlas_buffer->GetGPUVirtualAddress();
	tlasBuild.ScratchAccelerationStructureData = i_scratch_buffer->GetGPUVirtualAddress();

	ID3D12CommandList* i_command_buffer_0 = reinterpret_cast<DX_COMMAND_BUFFER&>(desc.command_buffer.get());
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> i_command_buffer;
	i_command_buffer_0->QueryInterface(IID_PPV_ARGS(&i_command_buffer));
	i_command_buffer->BuildRaytracingAccelerationStructure(&tlasBuild, 0, nullptr);

	// UAV barrier TLAS
	D3D12_RESOURCE_BARRIER tlas_barrier = {};
	tlas_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	tlas_barrier.UAV.pResource = i_tlas_buffer;

	i_command_buffer->ResourceBarrier(1, &tlas_barrier);

	i_tlas_buffer->AddRef();
	tlas_buffer_impl->set_current_state(resource_state_rt_bvh);

	DX_HEAP2* h2 = device_impl.get_resources_heap();
	ID3D12DescriptorHeap* heap = h2->Get();
	std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE> srv_handle = dx12_helpers_get_rw_descriptor_heap_handle(i_device.Get(), heap, desc.resource_slot);

	D3D12_SHADER_RESOURCE_VIEW_DESC srv = {};
	srv.ViewDimension =
		D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	srv.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv.RaytracingAccelerationStructure.Location =
		i_tlas_buffer->GetGPUVirtualAddress();
	static_cast<ID3D12Device*>(device_impl)->CreateShaderResourceView(
		nullptr,
		&srv,
		*srv_handle
	);
	return std::make_unique<DX_BVH_BUFFER2>(i_tlas_buffer, *srv_handle, resource_state_rt_bvh,
		resource_format_none, static_cast<size_t>(tlas_info.ResultDataMaxSizeInBytes));
}

void dx12_command_buffer_ray_trace2(RHI_DEVICE2& device, RHI_COMMAND_BUFFER& command_buffer,
	RHI_TEXTURE_2D2& render_target, RHI_RT_PIPELINE2& pipeline,
	RHI_BUFFER2& sbt) {

	ID3D12GraphicsCommandList* i_command_buffer_0 = static_cast<DX_COMMAND_BUFFER&>(command_buffer);
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList5> i_command_buffer;
	i_command_buffer_0->QueryInterface(IID_PPV_ARGS(&i_command_buffer));
	DX_DEVICE2& device_impl = static_cast<DX_DEVICE2&>(device);
	DX_SBT_BUFFER2& sbt_impl = static_cast<DX_SBT_BUFFER2&>(sbt);

	resource_state old_state_rt = render_target.get_current_state();
	D3D12_RESOURCE_BARRIER barrier = {};
	if (old_state_rt != resource_state_rt_render_target) {
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = render_target;
		barrier.Transition.StateBefore = dx12_resource_state_type[old_state_rt];
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		i_command_buffer->ResourceBarrier(1, &barrier);
		render_target.set_current_state(resource_state_rt_render_target);
	}

	const size_t shader_id_size = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	ID3D12Resource* i_table = static_cast<DX_BUFFER2&>(sbt);

	D3D12_DISPATCH_RAYS_DESC desc = {};

	desc.RayGenerationShaderRecord.StartAddress =
		i_table->GetGPUVirtualAddress() + sbt_impl.get_ray_gen_offset();
	desc.RayGenerationShaderRecord.SizeInBytes =
		sbt_impl.get_ray_gen_size();
	desc.MissShaderTable.StartAddress =
		i_table->GetGPUVirtualAddress() + sbt_impl.get_miss_offset();
	desc.MissShaderTable.SizeInBytes =
		sbt_impl.get_miss_size();
	desc.MissShaderTable.StrideInBytes =
		sbt_impl.get_record_size();
	desc.HitGroupTable.StartAddress =
		i_table->GetGPUVirtualAddress() + sbt_impl.get_hit_group_offset();
	desc.HitGroupTable.StrideInBytes =
		sbt_impl.get_record_size();
	desc.HitGroupTable.SizeInBytes = sbt_impl.get_hit_group_size();
	desc.Width = static_cast<UINT>(render_target.get_width());
	desc.Height = static_cast<UINT>(render_target.get_height());
	desc.Depth = 1;

	i_command_buffer->DispatchRays(&desc);

	D3D12_RESOURCE_BARRIER uav_barrier = {};
	uav_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uav_barrier.UAV.pResource = render_target;
	i_command_buffer->ResourceBarrier(1, &uav_barrier);
	if (old_state_rt != resource_state_rt_render_target) {
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = render_target;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.StateAfter = dx12_resource_state_type[old_state_rt];
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		i_command_buffer->ResourceBarrier(1, &barrier);
		render_target.set_current_state(old_state_rt);
	}
}

void dx12_command_buffer_copy_texture2(RHI_COMMAND_BUFFER& command_buffer, RHI_TEXTURE_2D2& dest_texture, RHI_TEXTURE_2D2& src_texture) {

	// src_texture:

	D3D12_RESOURCE_BARRIER barriers[2] = {};

	resource_state old_state_src = src_texture.get_current_state();
	barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barriers[0].Transition.pResource = static_cast<DX_TEXTURE_2D2&>(src_texture);
	barriers[0].Transition.StateBefore = dx12_resource_state_type[old_state_src];
	barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// back buffer:

	resource_state old_state_dest = dest_texture.get_current_state();
	barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barriers[1].Transition.pResource = static_cast<DX_TEXTURE_2D2&>(dest_texture);
	barriers[1].Transition.StateBefore = dx12_resource_state_type[old_state_dest];
	barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	ID3D12GraphicsCommandList* i_command_buffer = static_cast<DX_COMMAND_BUFFER&>(command_buffer);

	i_command_buffer->ResourceBarrier(2, barriers);
	src_texture.set_current_state(resource_state_copy_src);
	dest_texture.set_current_state(resource_state_copy_dest);

	i_command_buffer->CopyResource(
		static_cast<DX_TEXTURE_2D2&>(dest_texture),
		static_cast<DX_TEXTURE_2D2&>(src_texture)
	);

	barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barriers[0].Transition.StateAfter = dx12_resource_state_type[old_state_src];

	barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barriers[1].Transition.StateAfter = dx12_resource_state_type[old_state_dest];

	i_command_buffer->ResourceBarrier(2, barriers);
	src_texture.set_current_state(old_state_src);
	dest_texture.set_current_state(old_state_dest);
}

std::shared_ptr<RHI_TEXTURE_2D2> dx12_swap_chain_get_surface2(RHI_SWAP_CHAIN2& swap_chain, int surface_index) {

	ID3D12Resource* i_surface = nullptr;
	IDXGISwapChain3* i_swap_chain = static_cast<IDXGISwapChain3*>(swap_chain);
	if (surface_index == -1)
		surface_index = (int)i_swap_chain->GetCurrentBackBufferIndex();
	return swap_chain.get_render_target(surface_index);
}

void dx12_buffers_copy_buffer2(RHI_COMMAND_BUFFER& command_buffer, RHI_BUFFER2& src_buffer, RHI_BUFFER2& dest_buffer)
{
	static_cast<ID3D12GraphicsCommandList*>(static_cast<DX_COMMAND_BUFFER&>(command_buffer))->CopyResource(dest_buffer, src_buffer);
}

std::unique_ptr<D3D12_GPU_DESCRIPTOR_HANDLE> dx12_helpers_get_read_only_descriptor_heap_handle(ID3D12Device* device, ID3D12DescriptorHeap* heap, size_t slot) {

	D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
	if (slot + 1 > (int)desc.NumDescriptors) {
		throw std::exception("Max descriptors reached for type %d", desc.Type);
	}
	UINT rtvDescriptorSize =
		device->GetDescriptorHandleIncrementSize(desc.Type);
	auto h = heap->GetGPUDescriptorHandleForHeapStart();
	h.ptr += (slot * rtvDescriptorSize);
	return std::make_unique<D3D12_GPU_DESCRIPTOR_HANDLE>(h);
}

void dx12_render_pass_execute_raster_mode2(RHI_RENDER_PASS2& render_pass, RHI_COMMAND_BUFFER& command_buffer,
	fptr_render_pass_on_execute callback) {

	static float clearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f }; // RGBA

	RHI_DEPTH_BUFFER2* depth_buffer_impl = render_pass;
	RHI_GRAPHICS_PIPELINE2* pipeline_impl = render_pass;
	RHI_TEXTURE_2D2& render_target_impl = render_pass;
	DX_DEVICE2& device_impl = static_cast<DX_DEVICE2&>(static_cast<RHI_DEVICE2&>(render_pass));
	ID3D12GraphicsCommandList* i_command_buffer = static_cast<DX_COMMAND_BUFFER&>(command_buffer);

	RHI_VIEWPORT& vp = render_pass;
	D3D12_VIEWPORT dx_vp;
	dx_vp.TopLeftX = vp.x;
	dx_vp.TopLeftY = vp.y;
	dx_vp.Width = vp.width;
	dx_vp.Height = vp.height;
	dx_vp.MinDepth = vp.min_z;
	dx_vp.MaxDepth = vp.max_z;
	D3D12_RECT dx_scissor = {};
	dx_scissor.left = 0;
	dx_scissor.top = 0;
	dx_scissor.right = (LONG)vp.width;
	dx_scissor.bottom = (LONG)vp.height;

	resource_state old_state = render_target_impl.get_current_state();
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.pResource = render_target_impl;
	if (old_state != resource_state_raster_render_target) {
		barrier.Transition.StateBefore = dx12_resource_state_type[old_state];
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		i_command_buffer->ResourceBarrier(1, &barrier);
		render_target_impl.set_current_state(resource_state_raster_render_target);
	}


	D3D12_CPU_DESCRIPTOR_HANDLE& rtv_handle = static_cast<DX_TEXTURE_2D2&>(static_cast<RHI_TEXTURE_2D2&>(render_pass));
	D3D12_CPU_DESCRIPTOR_HANDLE* dsv_handle = nullptr;
	if (depth_buffer_impl) {

		auto rd = static_cast<DX_DEPTH_BUFFER2*>(depth_buffer_impl);
		dsv_handle = &static_cast<D3D12_CPU_DESCRIPTOR_HANDLE&>(*rd);
	}

	DX_RASTER_PIPELINE2& dx_pipeline_impl = reinterpret_cast<DX_RASTER_PIPELINE2&>(*pipeline_impl);
	ID3D12DescriptorHeap* resource_heap = *device_impl.get_resources_heap();
	auto sampler_heap_impl = device_impl.get_sampler_heap();
	ID3D12DescriptorHeap* sampler_heap = nullptr;

	if (sampler_heap_impl)
		sampler_heap = *sampler_heap_impl;
	ID3D12DescriptorHeap* heaps[] =
	{
		resource_heap,
		sampler_heap
	};
	i_command_buffer->SetDescriptorHeaps(sampler_heap ? 2 : 1, heaps);


	i_command_buffer->RSSetViewports(1, &dx_vp);
	i_command_buffer->RSSetScissorRects(1, &dx_scissor);

	i_command_buffer->OMSetRenderTargets(1, &rtv_handle, FALSE, dsv_handle);

	i_command_buffer->ClearRenderTargetView(
		rtv_handle,
		clearColor,
		0,
		nullptr
	);

	if (dsv_handle) {
		i_command_buffer->ClearDepthStencilView(
			*dsv_handle,                         // D3D12_CPU_DESCRIPTOR_HANDLE
			D3D12_CLEAR_FLAG_DEPTH,            // qué limpiar
			1.0f,                              // depth clear value
			0,                                 // stencil clear value
			0,                                 // num rects
			nullptr                            // rects
		);
	}

	// ASSERT HERE
	if (pipeline_impl) {



		i_command_buffer->SetPipelineState(dx_pipeline_impl);
		i_command_buffer->SetGraphicsRootSignature(static_cast<DX_PIPELINE_LAYOUT2&>(dx_pipeline_impl));

		ID3D12Device* i_device = static_cast<DX_DEVICE2&>(static_cast<RHI_DEVICE2&>(render_pass));
		auto resource_heap_handle = dx12_helpers_get_read_only_descriptor_heap_handle(i_device, resource_heap, 0);
		i_command_buffer->SetGraphicsRootDescriptorTable(0, *resource_heap_handle);

		if (sampler_heap) {
			auto sampler_heap_handle = dx12_helpers_get_read_only_descriptor_heap_handle(i_device, sampler_heap, 0);
			i_command_buffer->SetGraphicsRootDescriptorTable(1, *sampler_heap_handle);
		}
	}

	if (callback)
		callback();

	// leave render target in present mode
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	i_command_buffer->ResourceBarrier(1, &barrier);
	render_target_impl.set_current_state(resource_state_present);
}

