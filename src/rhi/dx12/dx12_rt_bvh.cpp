#include "dx12_rt_bvh.hpp"
#include "dx12_buffers.hpp"
#include "dx12_command_queue.hpp"
#include "dx12_command_buffer.hpp"

RHI_RT_BVH* dx12_rt_bvh_create(const RHI_RT_BVH_DESC* const desc)
{
	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);
	ASSERT_PTR(desc->command_buffer);
	ASSERT_PTR(desc->vertex_buffer);

	ID3D12Device* i_device_0 = *static_cast<DX_DEVICE*>(desc->device);
	ASSERT_PTR(i_device_0);
	ID3D12CommandList* i_command_buffer_0 = *static_cast<DX_COMMAND_BUFFER*>(desc->command_buffer);
	ASSERT_PTR(i_command_buffer_0);
	DX_BUFFER* vb_impl = static_cast<DX_BUFFER*>(desc->vertex_buffer);
	ASSERT_PTR(vb_impl);
	DX_BUFFER* ib_impl = static_cast<DX_BUFFER*>(desc->index_buffer);
	
	ID3D12Resource* i_vb = *vb_impl;
	ASSERT_PTR(i_vb);

	Microsoft::WRL::ComPtr<ID3D12Device5> i_device;
	ASSERT_SUCCESS(i_device_0->QueryInterface(IID_PPV_ARGS(&i_device)));
	ASSERT_PTR(i_device);

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> i_command_buffer;
	ASSERT_SUCCESS(i_command_buffer_0->QueryInterface(IID_PPV_ARGS(&i_command_buffer)));
	ASSERT_PTR(i_command_buffer);

	// create blas
	D3D12_RAYTRACING_GEOMETRY_DESC geomDesc = {};
	geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	geomDesc.Triangles.VertexBuffer.StartAddress =
		i_vb->GetGPUVirtualAddress();
	geomDesc.Triangles.VertexBuffer.StrideInBytes = static_cast<UINT64>(vb_impl->stride);
	geomDesc.Triangles.VertexCount = static_cast<UINT>(vb_impl->length / vb_impl->stride);
	geomDesc.Triangles.VertexFormat = dx12_resource_format_type[vb_impl->format];

	if (ib_impl)
	{
		ID3D12Resource *i_ib = *ib_impl;
		ASSERT_PTR(i_ib);
		geomDesc.Triangles.IndexBuffer =
			i_ib->GetGPUVirtualAddress();
		geomDesc.Triangles.IndexCount = static_cast<UINT>(ib_impl->length / ib_impl->stride);
		geomDesc.Triangles.IndexFormat = dx12_resource_format_type[ib_impl->format];
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

	RHI_BUFFER_DESC buffer_desc;
	buffer_desc.device = desc->device;
	buffer_desc.type = buffer_type_rt_bvh;
	buffer_desc.format = desc->vertex_buffer->format;
	buffer_desc.memory_type = buffer_memory_type_gpu_only;
	buffer_desc.length = blasInfo.ResultDataMaxSizeInBytes;
	buffer_desc.mips = 1;
	std::unique_ptr<DX_BUFFER> blas_buffer_impl;
	blas_buffer_impl.reset(dx12_buffers_create<DX_BUFFER>(&buffer_desc));
	buffer_desc.type = buffer_type_raw;
	buffer_desc.length = blasInfo.ScratchDataSizeInBytes;
	std::unique_ptr<DX_BUFFER> scratch_buffer_impl;
	scratch_buffer_impl.reset(dx12_buffers_create<DX_BUFFER>(&buffer_desc));

	ID3D12Resource *i_blas_buffer = *blas_buffer_impl;
	ASSERT_PTR(i_blas_buffer);
	ID3D12Resource *i_scratch_buffer = *scratch_buffer_impl;
	ASSERT_PTR(i_blas_buffer);

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
	DX_RT_BVH* result = new DX_RT_BVH();
	ASSERT_PTR(result);
	
	i_blas_buffer->AddRef();
	i_scratch_buffer->AddRef();
	result->set_handle(i_blas_buffer);
	result->scratch_handle.set_handle(i_scratch_buffer);
	return result;
}

RHI_BUFFER* dx12_rt_bvh_build_geometry_instances(const RT_GEOMETRY_INSTANCES_DESC* const desc)
{
	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);
	ASSERT_PTR(desc->command_buffer);
	ASSERT_PTR(desc->parent_bvh);

	DX_DEVICE* device_impl = static_cast<DX_DEVICE*>(desc->device);
	ID3D12Device* i_device_0 = *device_impl;
	ASSERT_PTR(i_device_0);
	
	ID3D12CommandList* i_command_buffer_0 = *static_cast<DX_COMMAND_BUFFER*>(desc->command_buffer);
	ASSERT_PTR(i_command_buffer_0);

	ID3D12Resource* i_blas_buffer = *static_cast<DX_RT_BVH*>(desc->parent_bvh);
	ASSERT_PTR(i_blas_buffer);

	Microsoft::WRL::ComPtr<ID3D12Device5> i_device;
	ASSERT_SUCCESS(i_device_0->QueryInterface(IID_PPV_ARGS(&i_device)));
	ASSERT_PTR(i_device);	
	
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> i_command_buffer;
	ASSERT_SUCCESS(i_command_buffer_0->QueryInterface(IID_PPV_ARGS(&i_command_buffer)));
	ASSERT_PTR(i_command_buffer);

	// TLAS
	RHI_BUFFER_DESC inputs_buffer_desc;
	inputs_buffer_desc.device = desc->device;
	inputs_buffer_desc.type = buffer_type_raw;
	inputs_buffer_desc.length = desc->instance_count * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);
	inputs_buffer_desc.memory_type = buffer_memory_type_shared_rw;
	inputs_buffer_desc.mips = 1;
	std::unique_ptr<DX_BUFFER> tlas_inputs_buffer_impl;
	tlas_inputs_buffer_impl.reset(dx12_buffers_create<DX_BUFFER>(&inputs_buffer_desc));
	ID3D12Resource* i_tlas_inputs_buffer = *tlas_inputs_buffer_impl;
	ASSERT_PTR(i_tlas_inputs_buffer);

	D3D12_RAYTRACING_INSTANCE_DESC* instances = nullptr;
	D3D12_RANGE readRange(0, 0);

	i_tlas_inputs_buffer->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&instances));

	UINT iid = 0;
	for (size_t i = 0; i < desc->instance_count; i++)
	{
		D3D12_RAYTRACING_INSTANCE_DESC& instance = instances[i];
		instance.InstanceID = iid++;
		instance.InstanceMask = 0xFF;
		instance.AccelerationStructure = i_blas_buffer->GetGPUVirtualAddress();

		auto& mat = desc->transforms[i];

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
	tlas_inputs.Flags =
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	if (!desc->read_only) 
		tlas_inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO tlas_info = {};
	i_device->GetRaytracingAccelerationStructurePrebuildInfo(&tlas_inputs, &tlas_info);

	RHI_BUFFER_DESC buffer_desc;
	buffer_desc.device = desc->device;
	buffer_desc.type = buffer_type_rt_bvh;
	buffer_desc.memory_type = buffer_memory_type_gpu_only;
	buffer_desc.length = tlas_info.ResultDataMaxSizeInBytes;
	buffer_desc.mips = 1;
	std::unique_ptr<DX_BUFFER> tlas_buffer_impl;
	tlas_buffer_impl.reset(dx12_buffers_create<DX_BUFFER>(&buffer_desc));
	buffer_desc.type = buffer_type_raw;
	buffer_desc.length = tlas_info.ScratchDataSizeInBytes;
	std::unique_ptr<DX_BUFFER> scratch_buffer_impl;
	scratch_buffer_impl.reset(dx12_buffers_create<DX_BUFFER>(&buffer_desc));

	ID3D12Resource* i_tlas_buffer = *tlas_buffer_impl;
	ASSERT_PTR(i_tlas_buffer);
	ID3D12Resource* i_scratch_buffer = *scratch_buffer_impl;
	ASSERT_PTR(i_scratch_buffer);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasBuild = {};
	tlasBuild.Inputs = tlas_inputs;
	tlasBuild.DestAccelerationStructureData = i_tlas_buffer->GetGPUVirtualAddress();
	tlasBuild.ScratchAccelerationStructureData = i_scratch_buffer->GetGPUVirtualAddress();

	i_command_buffer->BuildRaytracingAccelerationStructure(&tlasBuild, 0, nullptr);

	// UAV barrier TLAS
	D3D12_RESOURCE_BARRIER tlas_barrier = {};
	tlas_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	tlas_barrier.UAV.pResource = i_tlas_buffer;
	i_command_buffer->ResourceBarrier(1, &tlas_barrier);

	//DX_RESOURCE_HEAP_DESCRIPTOR srv_handle = dx12_helpers_get_next_descriptor_heap_handle(device_impl, heap_id_type_resources);

	// CREAR VIEW EN OTRO LADO
	//D3D12_SHADER_RESOURCE_VIEW_DESC srv = {};
	//srv.ViewDimension =
	//	D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	//srv.Shader4ComponentMapping =
	//	D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//srv.RaytracingAccelerationStructure.Location =
	//	i_tlas_buffer->GetGPUVirtualAddress();
	//i_device->CreateShaderResourceView(
	//	nullptr,
	//	&srv,
	//	srv_handle.cpu_descriptor_handle
	//);
	DX_BVH_BUFFER* result = new DX_BVH_BUFFER();
	ASSERT_PTR(result);
	i_tlas_buffer->AddRef();
	i_scratch_buffer->AddRef();
	i_tlas_inputs_buffer->AddRef();
	result->set_handle(i_tlas_buffer);
	result->scratch_handle.set_handle(i_scratch_buffer);
	result->inputs_buffer_handle.set_handle(i_tlas_inputs_buffer);
	return result;
}