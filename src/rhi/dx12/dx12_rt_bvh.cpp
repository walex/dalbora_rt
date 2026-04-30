#include "dx12_rt_bvh.hpp"
#include "dx12_buffers.hpp"
#include "dx12_command_queue.hpp"

std::unique_ptr<RHI_OBJECT> dx12_rt_bvh_create(const RHI_RT_BVH_DESC& desc) {

	auto device = com_query_interface<ID3D12Device5>(desc.device());
	auto command_list = com_query_interface<ID3D12GraphicsCommandList4>(desc.command_buffer());

	auto& vb_h = reinterpret_cast<RHI_BUFFER_RESOURCE&>(desc.geometry_buffer.get().get_vertex_buffer());
	auto ib_h = dynamic_cast<RHI_BUFFER_RESOURCE*>(desc.geometry_buffer.get().get_index_buffer());
	auto& transforms = desc.geometry_buffer.get().get_transforms();
	ID3D12Resource* vertexBuffer = vb_h.handle<DX_RESOURCE_HANDLE>();

	// create geometry descriptor
	D3D12_RAYTRACING_GEOMETRY_DESC geomDesc = {};
	geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	geomDesc.Triangles.VertexBuffer.StartAddress =
		vertexBuffer->GetGPUVirtualAddress();

	geomDesc.Triangles.VertexBuffer.StrideInBytes = vb_h.stride;

		geomDesc.Triangles.VertexCount = (UINT)(vb_h.size / vb_h.stride);

	geomDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;

	if (ib_h) {

		ID3D12Resource* indexBuffer  = ib_h->handle<DX_RESOURCE_HANDLE>();
		geomDesc.Triangles.IndexBuffer =
			indexBuffer->GetGPUVirtualAddress();
		geomDesc.Triangles.IndexCount = (UINT)(ib_h->size / ib_h->stride);
		geomDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
	}
	// opcional
	geomDesc.Triangles.Transform3x4 = 0;

	// BLAS
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.NumDescs = 1;
	inputs.pGeometryDescs = &geomDesc;

	// pre build info
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO blasInfo = {};

	device->GetRaytracingAccelerationStructurePrebuildInfo(
		&inputs,
		&blasInfo
	);

	RHI_BUFFER_DESC buffer_desc(desc.device);
	buffer_desc.initial_state = resource_state_none;
	buffer_desc.is_uav = true;
	buffer_desc.memory_type = buffer_memory_type_gpu_only;
	buffer_desc.size = blasInfo.ResultDataMaxSizeInBytes;
	auto blasBuffer = dx12_buffers_create(buffer_desc);
	buffer_desc.size = blasInfo.ScratchDataSizeInBytes;
	auto scratchBuffer = dx12_buffers_create(buffer_desc);

	ID3D12Resource* iblasBuffer = blasBuffer->handle<DX_RESOURCE_HANDLE>();
	ID3D12Resource* iscratchBuffer = scratchBuffer->handle<DX_RESOURCE_HANDLE>();

	// build
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};

	buildDesc.Inputs = inputs;
	buildDesc.ScratchAccelerationStructureData = iscratchBuffer->GetGPUVirtualAddress();
	buildDesc.DestAccelerationStructureData = iblasBuffer->GetGPUVirtualAddress();

	command_list->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

	// UAV barrier BLAS
	D3D12_RESOURCE_BARRIER blasBarrier = {};
	blasBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	blasBarrier.UAV.pResource = iblasBuffer;

	command_list->ResourceBarrier(1, &blasBarrier);

	// TLAS
	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instances(transforms.size());
	for (auto i = 0; i < transforms.size(); i++) {
		
		auto& instance = instances[i];
		auto& mat = transforms[i];

		instance.InstanceMask = 0xFF;
		instance.AccelerationStructure = iblasBuffer->GetGPUVirtualAddress();
		
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

	buffer_desc.initial_state = resource_state_generic_read;
	buffer_desc.is_uav = false;
	buffer_desc.memory_type = buffer_memory_type_gpu_only;
	buffer_desc.size = instances.size() * sizeof(instances[0]);
	auto instanceBuffer = dx12_buffers_create(buffer_desc);
	ID3D12Resource* iinstanceBuffer = instanceBuffer->handle<DX_RESOURCE_HANDLE>();

	void* mapped = nullptr;
	iinstanceBuffer->Map(0, nullptr, &mapped);
	memcpy(mapped, instances.data(), buffer_desc.size);
	iinstanceBuffer->Unmap(0, nullptr);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInputs = {};
	tlasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	tlasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	tlasInputs.NumDescs = 1;
	tlasInputs.InstanceDescs = iinstanceBuffer->GetGPUVirtualAddress();

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO tlasInfo = {};
	device->GetRaytracingAccelerationStructurePrebuildInfo(&tlasInputs, &tlasInfo);

	buffer_desc.initial_state = resource_state_none;
	buffer_desc.is_uav = true;
	buffer_desc.memory_type = buffer_memory_type_gpu_only;
	buffer_desc.size = tlasInfo.ResultDataMaxSizeInBytes;
	auto tlasBuffer = dx12_buffers_create(buffer_desc);
	buffer_desc.size = tlasInfo.ScratchDataSizeInBytes;
	auto tlasScratch = dx12_buffers_create(buffer_desc);

	ID3D12Resource* itlasBuffer = tlasBuffer->handle<DX_RESOURCE_HANDLE>();
	iscratchBuffer = tlasScratch->handle<DX_RESOURCE_HANDLE>();

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasBuild = {};
	tlasBuild.Inputs = tlasInputs;
	tlasBuild.DestAccelerationStructureData = itlasBuffer->GetGPUVirtualAddress();
	tlasBuild.ScratchAccelerationStructureData = iscratchBuffer->GetGPUVirtualAddress();

	command_list->BuildRaytracingAccelerationStructure(&tlasBuild, 0, nullptr);

	// UAV barrier TLAS
	D3D12_RESOURCE_BARRIER tlasBarrier = {};
	tlasBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	tlasBarrier.UAV.pResource = itlasBuffer;

	command_list->ResourceBarrier(1, &tlasBarrier);

	dx12_command_queue_execute_synchronized(desc.command_queue.get(), desc.command_buffer());

	return std::make_unique<RHI_RESOURCE>(new DX_RESOURCE_HANDLE(itlasBuffer));
}