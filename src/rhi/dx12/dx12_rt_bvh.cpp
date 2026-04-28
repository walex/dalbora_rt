#include "dx12_rt_bvh.hpp"
#include "dx12_buffers.hpp"
#include "dx12_command_queue.hpp"

std::unique_ptr<RHI_OBJECT> dx12_rt_bvh_create(const RHI_RT_BVH_DESC& bvh_desc) {

	auto device = dx_rhi_get_interface<ID3D12Device5>(*bvh_desc.device);
	auto command_list = dx_rhi_get_interface<ID3D12GraphicsCommandList4>(*bvh_desc.command_buffer);

	auto vb_h = dynamic_cast<RHI_BUFFER_RESOURCE*>(bvh_desc.geometry_buffer->get_vertex_buffer());
	auto ib_h = dynamic_cast<RHI_BUFFER_RESOURCE*>(bvh_desc.geometry_buffer->get_index_buffer());
	auto& transforms = bvh_desc.geometry_buffer->get_transforms();
	auto vertexBuffer = dx_rhi_get_interface<ID3D12Resource>(*vb_h);
	
	// create geometry descriptor
	D3D12_RAYTRACING_GEOMETRY_DESC geomDesc = {};
	geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	geomDesc.Triangles.VertexBuffer.StartAddress =
		vertexBuffer->GetGPUVirtualAddress();

	geomDesc.Triangles.VertexBuffer.StrideInBytes = vb_h->stride;

		geomDesc.Triangles.VertexCount = vb_h->size / vb_h->stride;

	geomDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;

	if (ib_h) {
		auto indexBuffer = dx_rhi_get_interface<ID3D12Resource>(*ib_h);
		geomDesc.Triangles.IndexBuffer =
			indexBuffer->GetGPUVirtualAddress();
		geomDesc.Triangles.IndexCount = ib_h->size / ib_h->stride;
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

	RHI_BUFFER_DESC desc;
	desc.initial_state = resource_state_none;
	desc.is_uav = true;
	desc.device = bvh_desc.device;
	desc.memory_type = buffer_memory_type_gpu_only;
	desc.size = blasInfo.ResultDataMaxSizeInBytes;
	auto blasBuffer = dx_rhi_get_interface<ID3D12Resource>(*dx12_buffers_create(desc).get());
	desc.size = blasInfo.ScratchDataSizeInBytes;
	auto scratchBuffer = dx_rhi_get_interface<ID3D12Resource>(*dx12_buffers_create(desc).get());

	// build
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};

	buildDesc.Inputs = inputs;
	buildDesc.ScratchAccelerationStructureData = scratchBuffer->GetGPUVirtualAddress();
	buildDesc.DestAccelerationStructureData = blasBuffer->GetGPUVirtualAddress();

	command_list->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

	// UAV barrier BLAS
	D3D12_RESOURCE_BARRIER blasBarrier = {};
	blasBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	blasBarrier.UAV.pResource = blasBuffer.Get();

	command_list->ResourceBarrier(1, &blasBarrier);

	// TLAS
	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instances(transforms.size());
	for (auto i = 0; i < transforms.size(); i++) {
		
		auto& instance = instances[i];
		auto& mat = transforms[i];

		instance.InstanceMask = 0xFF;
		instance.AccelerationStructure = blasBuffer->GetGPUVirtualAddress();
		
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

	desc.initial_state = resource_state_generic_read;
	desc.is_uav = false;
	desc.memory_type = buffer_memory_type_gpu_only;
	desc.size = instances.size() * sizeof(instances[0]);
	auto instanceBuffer = dx_rhi_get_interface<ID3D12Resource>(*dx12_buffers_create(desc).get());

	void* mapped = nullptr;
	instanceBuffer->Map(0, nullptr, &mapped);
	memcpy(mapped, instances.data(), desc.size);
	instanceBuffer->Unmap(0, nullptr);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInputs = {};
	tlasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	tlasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	tlasInputs.NumDescs = 1;
	tlasInputs.InstanceDescs = instanceBuffer->GetGPUVirtualAddress();

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO tlasInfo = {};
	device->GetRaytracingAccelerationStructurePrebuildInfo(&tlasInputs, &tlasInfo);

	desc.initial_state = resource_state_none;
	desc.is_uav = true;
	desc.memory_type = buffer_memory_type_gpu_only;
	desc.size = tlasInfo.ResultDataMaxSizeInBytes;
	auto tlasBuffer = dx_rhi_get_interface<ID3D12Resource>(*dx12_buffers_create(desc).get());
	desc.size = tlasInfo.ScratchDataSizeInBytes;
	auto tlasScratch = dx_rhi_get_interface<ID3D12Resource>(*dx12_buffers_create(desc).get());

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasBuild = {};
	tlasBuild.Inputs = tlasInputs;
	tlasBuild.DestAccelerationStructureData = tlasBuffer->GetGPUVirtualAddress();
	tlasBuild.ScratchAccelerationStructureData = tlasScratch->GetGPUVirtualAddress();

	command_list->BuildRaytracingAccelerationStructure(&tlasBuild, 0, nullptr);

	// UAV barrier TLAS
	D3D12_RESOURCE_BARRIER tlasBarrier = {};
	tlasBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	tlasBarrier.UAV.pResource = tlasBuffer.Get();

	command_list->ResourceBarrier(1, &tlasBarrier);

	dx12_command_queue_execute_single_command_buffer_synchronized(*bvh_desc.command_queue, *bvh_desc.command_buffer);

	return std::make_unique<RHI_RESOURCE>(new DX_BUFFER_HANDLE(tlasBuffer.Detach()));
}