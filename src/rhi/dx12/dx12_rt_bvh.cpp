#include "dx12_rt_bvh.hpp"
#include "dx12_buffers.hpp"
#include "dx12_command_queue.hpp"

std::unique_ptr<RHI_RT_BVH> dx12_rt_bvh_create(const RHI_RT_BVH_DESC &desc)
{

	ID3D12Device *i_device_0 = static_cast<DX_DEVICE&>(desc.device.get());
	ID3D12CommandList *i_command_buffer_0 = static_cast<DX_COMMAND_BUFFER&>(desc.command_buffer.get());
	DX_VERTEX_BUFFER& vb_impl = static_cast<DX_VERTEX_BUFFER&>(desc.vertex_buffer.get());
	DX_INDEX_BUFFER* ib_impl = static_cast<DX_INDEX_BUFFER*>(desc.index_buffer.get());
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

		ID3D12Resource *i_ib = *ib_impl;
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

	RHI_BUFFER_DESC buffer_desc(desc.device);
	buffer_desc.initial_state = resource_state_rt_bvh;
	buffer_desc.type = buffer_type_rt_bvh;
	buffer_desc.format = desc.vertex_buffer.get().get_format();
	buffer_desc.memory_type = buffer_memory_type_gpu_only;
	buffer_desc.length = blasInfo.ResultDataMaxSizeInBytes;
	auto blas_buffer_impl = dx12_buffers_create_raw(buffer_desc);
	buffer_desc.length = blasInfo.ScratchDataSizeInBytes;
	buffer_desc.type = buffer_type_raw;
	auto scratch_buffer_impl = dx12_buffers_create_raw(buffer_desc);

	ID3D12Resource *i_blas_buffer = *blas_buffer_impl;
	i_blas_buffer->AddRef();
	ID3D12Resource *i_scratch_buffer = *scratch_buffer_impl;

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

	return std::make_unique<DX_RT_BVH>(i_blas_buffer);
}

void dx12_bvh_build_geometry_instances(const RT_GEOMETRY_INSTANCES_DESC &desc)
{

	ID3D12Resource *i_blas_buffer = static_cast<DX_RT_BVH &>(desc.parent_bvh.get());
	DX_DEVICE &device_impl = reinterpret_cast<DX_DEVICE &>(desc.device.get());

	// TLAS

	RHI_BUFFER_DESC inputs_buffer_desc(device_impl);
	inputs_buffer_desc.length = desc.transforms.size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);
	inputs_buffer_desc.memory_type = buffer_memory_type_shared_rw;
	inputs_buffer_desc.initial_state = resource_state_generic_read;
	auto tlas_inputs_buffer_impl = dx12_buffers_create_raw(inputs_buffer_desc);
	ID3D12Resource *i_tlas_inputs_buffer = static_cast<DX_BUFFER &>(*tlas_inputs_buffer_impl);

	// size_t mapped_size = buffer_desc.width * buffer_desc.height;
	// void* mapped = nullptr;
	// i_tlas_inputs_buffer->Map(0, nullptr, &mapped);
	// memcpy(mapped, instances.data(), mapped_size);
	// i_tlas_inputs_buffer->Unmap(0, nullptr);

	D3D12_RAYTRACING_INSTANCE_DESC *instances = nullptr;
	D3D12_RANGE readRange(0, 0);

	i_tlas_inputs_buffer->Map(
		0,
		nullptr,
		reinterpret_cast<void **>(&instances));

	UINT iid = 0;
	for (size_t i = 0; i < desc.transforms.size(); i++)
	{

		D3D12_RAYTRACING_INSTANCE_DESC &instance = instances[i];
		instance.InstanceID = iid++;
		instance.InstanceMask = 0xFF;
		instance.AccelerationStructure = i_blas_buffer->GetGPUVirtualAddress();

		auto &mat = desc.transforms[i];

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
	static_cast<ID3D12Device *>(device_impl)->QueryInterface(IID_PPV_ARGS(&i_device));
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO tlas_info = {};
	i_device->GetRaytracingAccelerationStructurePrebuildInfo(&tlas_inputs, &tlas_info);

	RHI_BUFFER_DESC buffer_desc(desc.device);
	buffer_desc.initial_state = resource_state_rt_bvh;
	buffer_desc.type = buffer_type_rt_bvh;
	// buffer_desc.format = desc.vertex_buffer.get().get_format();
	buffer_desc.memory_type = buffer_memory_type_gpu_only;
	buffer_desc.length = tlas_info.ResultDataMaxSizeInBytes;
	auto tlas_buffer_impl = dx12_buffers_create_raw(buffer_desc);
	buffer_desc.length = tlas_info.ScratchDataSizeInBytes;
	buffer_desc.type = buffer_type_raw;
	auto scratch_buffer_impl = dx12_buffers_create_raw(buffer_desc);

	//////////////////////////////////////////////

	// buffer_desc.base_state = resource_state_none;
	// buffer_desc.memory_type = buffer_memory_type_gpu_only;
	// buffer_desc.width = tlas_info.ResultDataMaxSizeInBytes;
	// auto tlas_buffer_impl = dx12_buffers_create_raw(buffer_desc);
	// buffer_desc.width = tlas_info.ScratchDataSizeInBytes;
	// auto scratch_buffer_impl = dx12_buffers_create_raw(buffer_desc);

	///////////////////////////////////

	ID3D12Resource *i_tlas_buffer = *tlas_buffer_impl;
	ID3D12Resource *i_scratch_buffer = *scratch_buffer_impl;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasBuild = {};
	tlasBuild.Inputs = tlas_inputs;
	tlasBuild.DestAccelerationStructureData = i_tlas_buffer->GetGPUVirtualAddress();
	tlasBuild.ScratchAccelerationStructureData = i_scratch_buffer->GetGPUVirtualAddress();

	ID3D12CommandList *i_command_buffer_0 = reinterpret_cast<DX_COMMAND_BUFFER &>(desc.command_buffer.get());
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> i_command_buffer;
	i_command_buffer_0->QueryInterface(IID_PPV_ARGS(&i_command_buffer));
	i_command_buffer->BuildRaytracingAccelerationStructure(&tlasBuild, 0, nullptr);

	// UAV barrier TLAS
	D3D12_RESOURCE_BARRIER tlas_barrier = {};
	tlas_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	tlas_barrier.UAV.pResource = i_tlas_buffer;

	i_command_buffer->ResourceBarrier(1, &tlas_barrier);
}
