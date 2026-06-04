#include "dx12_helpers.hpp"
#include "dx12_rhi.hpp"

resource_format dx12_helpers_resource_format_from_dxgi_format(const DXGI_FORMAT format) {

    std::span<const DXGI_FORMAT> s(dx12_resource_format_type);
    auto it = std::find(s.begin(), s.end(), format);
    if (it == s.end())
    {
        throw std::exception("texture format no supported");
    }
    return static_cast<resource_format>(std::distance(s.begin(), it));
}

void dx12_helpers_copy_4x4Matrix_to_rt_instance(const float*const* transforms, const size_t instance_count, D3D12_GPU_VIRTUAL_ADDRESS gpu_mem, ID3D12Resource* i_buffer) {

    D3D12_RAYTRACING_INSTANCE_DESC* instances = nullptr;

    i_buffer->Map(
        0,
        nullptr,
        reinterpret_cast<void**>(&instances));

    UINT iid = 0;
    for (size_t i = 0; i < instance_count; i++)
    {
        float* transform_values = const_cast<float*>(transforms[i]);
        D3D12_RAYTRACING_INSTANCE_DESC& instance = instances[i];
        instance.InstanceID = iid++;
        instance.InstanceMask = 0xFF;
        instance.AccelerationStructure = gpu_mem;
        instance.Transform[0][0] = *(transform_values++);
        instance.Transform[0][1] = *(transform_values++);
        instance.Transform[0][2] = *(transform_values++);
        instance.Transform[0][3] = *(transform_values++);

        // fila 1
        instance.Transform[1][0] = *(transform_values++);
        instance.Transform[1][1] = *(transform_values++);
        instance.Transform[1][2] = *(transform_values++);
        instance.Transform[1][3] = *(transform_values++);

        // fila 2
        instance.Transform[2][0] = *(transform_values++);
        instance.Transform[2][1] = *(transform_values++);
        instance.Transform[2][2] = *(transform_values++);
        instance.Transform[2][3] = *(transform_values++);

        //dx12_helpers_copy_eigen_matrix_to_array(
        //    transforms[i],
        //    instance.Transform

        //);
    }

    i_buffer->Unmap(0, nullptr);
}