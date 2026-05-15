#include "test_api.hpp"
#include "rhi.hpp"
#include "dx12_rhi.hpp"
#include "dx12_rt_bvh.hpp"

void test_rt_triangle(fptr_test_on_init on_init,
	fptr_test_on_draw on_draw,
	fptr_test_on_end on_end,
	fptr_test_on_layout on_layout)
{

	std::unique_ptr<RHI_BUFFER> vertex_buffer;
	std::unique_ptr<RHI_BUFFER> index_buffer;

	float aspect = 800.0f / 600.0f;
	float x = 0.5f;

	struct Vertex
	{
		float x, y, z;
	};

	Vertex vertices[] =
	{
		{0.0f, x, 0.0f}, // top
		{x, -x, 0.0f},	 // right
		{-x, -x, 0.0f}	 // left
	};
	constexpr unsigned int vertex_count = sizeof(vertices) / sizeof(Vertex);

	uint16_t indices[] =
	{
		0, 1, 2 };
	constexpr unsigned int index_count = sizeof(indices) / sizeof(uint16_t);

	test_swap_chain([&](RHI_DEVICE& device, RHI_COMMAND_QUEUE& command_queue,
		RHI_COMMAND_BUFFER& command_buffer)
		{
			// create geometry buffers
			RHI_VERTEX_BUFFER_DESC vb_desc(device);
			vb_desc.count = vertex_count;
			vb_desc.length = vertex_count * sizeof(Vertex);
			vb_desc.stride = sizeof(Vertex);
			vb_desc.format = resource_format_float3;
			vertex_buffer = rhi_vertex_buffer_create(vb_desc);

			RHI_INDEX_BUFFER_DESC ib_desc(device);
			ib_desc.count = index_count;
			ib_desc.length = sizeof(uint16_t) * index_count;
			ib_desc.stride = sizeof(uint16_t);
			ib_desc.format = resource_format_uint16;
			index_buffer = rhi_index_buffer_create(ib_desc);

			std::unique_ptr<RHI_RT_BVH> bvh;
			rhi_command_queue_execute(command_queue, true, [&](RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
				std::vector<RHI_COMMAND_BUFFER*>& command_buffer_list) {

					rhi_command_buffer_record(command_buffer, [&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl)) {

						RHI_RT_BVH_DESC blas_desc(device, command_buffer, *vertex_buffer, index_buffer.get());
						bvh = dx12_rt_bvh_create(blas_desc);

						RT_GEOMETRY_INSTANCES_DESC tlas_desc(device, command_buffer, *bvh);
						tlas_desc.transforms = { Eigen::Matrix4f::Identity(), Eigen::Matrix4f::Identity() };
						dx12_bvh_build_geometry_instances(tlas_desc);
					});
					command_buffer_list.push_back(&command_buffer);
				});
			return;
		});
}