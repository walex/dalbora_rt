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

void dx12_helpers_copy_eigen_matrix_to_array(const Eigen::Matrix4f& mat, float array[4][4]) {
    // fila 0
    array[0][0] = mat(0, 0);
    array[0][1] = mat(0, 1);
    array[0][2] = mat(0, 2);
    array[0][3] = mat(0, 3);

    // fila 1
    array[1][0] = mat(1, 0);
    array[1][1] = mat(1, 1);
    array[1][2] = mat(1, 2);
    array[1][3] = mat(1, 3);

    // fila 2
    array[2][0] = mat(2, 0);
    array[2][1] = mat(2, 1);
    array[2][2] = mat(2, 2);
    array[2][3] = mat(2, 3);
}

void dx12_helpers_copy_eigen_matrices_to_resource(const Eigen::Matrix4f* transforms, const size_t instance_count, D3D12_GPU_VIRTUAL_ADDRESS gpu_mem, ID3D12Resource* i_buffer) {

    D3D12_RAYTRACING_INSTANCE_DESC* instances = nullptr;

    i_buffer->Map(
        0,
        nullptr,
        reinterpret_cast<void**>(&instances));

    UINT iid = 0;
    for (size_t i = 0; i < instance_count; i++)
    {
        D3D12_RAYTRACING_INSTANCE_DESC& instance = instances[i];
        instance.InstanceID = iid++;
        instance.InstanceMask = 0xFF;
        instance.AccelerationStructure = gpu_mem;
        auto& mat = transforms[i];
        dx12_helpers_copy_eigen_matrix_to_array(
            mat,
            instance.Transform

        );
    }

    i_buffer->Unmap(0, nullptr);
}