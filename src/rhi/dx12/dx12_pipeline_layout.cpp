#include "dx12_pipeline_layout.hpp"
#include "dx12_helpers.hpp"

RHI_PIPELINE_LAYOUT* dx12_pipeline_layout_create(const RHI_PIPELINE_LAYOUT_DESC* const desc) {

	ASSERT_NULL(desc);
	ASSERT_NULL(desc->device);

	ID3D12Device* i_device = *static_cast<DX_DEVICE*>(desc->device);
	ASSERT_NULL(i_device);

	std::vector<D3D12_DESCRIPTOR_RANGE1> descriptor_ranges;
	std::vector<D3D12_DESCRIPTOR_RANGE1> descriptor_ranges_sampler;

	for (size_t i = 0; i < MAX_PIPELINE_DESCRIPTORS; i++) {

		if (desc->descriptors[i].pool_range_start == -1 
			|| desc->descriptors[i].pool_range_count == -1) {
			break;
		}
		const RHI_DESCRIPTOR_DESC& descriptor = desc->descriptors[i];
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
		root_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}
	if (descriptor_ranges_sampler.size() > 0) {

		D3D12_ROOT_PARAMETER1& root_parameter = root_params.emplace_back();
		root_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		root_parameter.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(descriptor_ranges_sampler.size());
		root_parameter.DescriptorTable.pDescriptorRanges = descriptor_ranges_sampler.data();
		root_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
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

	ASSERT_FAILED(D3D12SerializeVersionedRootSignature(
		&rootDesc,
		&signature,
		&error
	));
	ASSERT_NULL(error);
	ASSERT_NULL(signature);
	ID3D12RootSignature* i_root_signature = nullptr;
	ASSERT_FAILED(i_device->CreateRootSignature(
		0,
		signature->GetBufferPointer(),
		signature->GetBufferSize(),
		IID_PPV_ARGS(&i_root_signature)
	));
	ASSERT_NULL(i_root_signature);
	DX_PIPELINE_LAYOUT* result = new DX_PIPELINE_LAYOUT();
	result->set_handle(i_root_signature);
	return result;
}