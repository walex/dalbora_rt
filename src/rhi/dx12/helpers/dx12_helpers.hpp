#ifndef __dx12_helpers_hpp__
#define __dx12_helpers_hpp__

#include "platform.hpp"
#include "rhi.hpp"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <wrl.h>

resource_format dx12_helpers_resource_format_from_dxgi_format(const DXGI_FORMAT format);
void dx12_helpers_copy_eigen_matrix_to_array(const Eigen::Matrix4f& mat, float d3d12_mat[4][4]);
void dx12_helpers_copy_eigen_matrices_to_resource(const Eigen::Matrix4f* transforms,
	const size_t instance_count, D3D12_GPU_VIRTUAL_ADDRESS gpu_mem, 
	ID3D12Resource* i_buffer);
#endif