#include "test_api.hpp"
#include "rhi.hpp"
#include "dx12_rhi.hpp"

#include "SimpleCamera.h"

using Mat4 = Eigen::Matrix4f;
using Vec4 = Eigen::Vector4f;
using Vec3 = Eigen::Vector3f;

struct alignas(256) CameraCB
{
	Mat4 view;
	Mat4 projection;
};

struct alignas(256) ObjectCB
{
	Mat4 world = Mat4::Identity();
};


Eigen::Matrix4f XMMatrixToEigen(const XMMATRIX& xm)
{
	XMFLOAT4X4 temp;
	XMStoreFloat4x4(&temp, xm);

	Eigen::Matrix4f e;

	e <<
		temp._11, temp._12, temp._13, temp._14,
		temp._21, temp._22, temp._23, temp._24,
		temp._31, temp._32, temp._33, temp._34,
		temp._41, temp._42, temp._43, temp._44;

	return e;
}

void get_transforms(Mat4& w, Mat4& v, Mat4& p) {

	XMMATRIX world = XMMatrixIdentity();
	//	XMMatrixRotationY(XMConvertToRadians(45.0f));

	XMVECTOR eye =
		XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);

	XMVECTOR target =
		XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	XMVECTOR up =
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view =
		XMMatrixLookAtLH(eye, target, up);

	XMMATRIX projection =
		XMMatrixPerspectiveFovLH(
			XM_PIDIV4,
			800.0f / 600.0f,
			0.1f,
			100.0f);

	w = XMMatrixToEigen(XMMatrixTranspose(world * view * projection));
	v = XMMatrixToEigen(view);
	p = XMMatrixToEigen(projection);
}

struct Transform
{
	Vec3 position = Vec3(0, 0, 0);

	Eigen::Quaternionf rotation =
		Eigen::Quaternionf::Identity();

	Vec3 scale = Vec3(1, 1, 1);

	Vec3 pivot = Vec3(0, 0, 0);
};

void rotate_transform(
	Eigen::Quaternionf& rotation,
	float dt,
	float radians_per_second,
	const Vec3& axis)
{
	float angle =
		radians_per_second * dt;

	Eigen::Quaternionf q(
		Eigen::AngleAxisf(
			angle,
			axis.normalized()));

	rotation =
		(rotation * q).normalized();
}
Mat4 build_matrix(
	const Vec3& position,
	const Eigen::Quaternionf& rotation,
	const Vec3& scale,
	const Vec3& pivot)
{
	Eigen::Affine3f T =
		Eigen::Affine3f::Identity();

	T.translate(position);

	Eigen::Affine3f P1 =
		Eigen::Affine3f::Identity();

	P1.translate(-pivot);

	Eigen::Affine3f P2 =
		Eigen::Affine3f::Identity();

	P2.translate(pivot);

	Eigen::Affine3f R =
		Eigen::Affine3f::Identity();

	R.rotate(rotation);

	Eigen::Affine3f S =
		Eigen::Affine3f::Identity();

	S.scale(scale);

	Eigen::Affine3f M =
		T * P2 * R * S * P1;

	return M.matrix();
}

float get_delta_time()
{
	using clock = std::chrono::high_resolution_clock;

	static auto previous_time = clock::now();

	auto current_time = clock::now();

	std::chrono::duration<float> delta =
		current_time - previous_time;

	previous_time = current_time;

	return delta.count();
}

void test_raster_triangle(std::function<void(RHI_DEVICE& device)> UNUSED_PARAM(on_init)
	, std::function<void(RHI_RENDER_PASS& render_pass)> UNUSED_PARAM(on_draw)
	, std::function<void(RHI_DEVICE& device)> UNUSED_PARAM(on_end)) {

	std::unique_ptr<RHI_RASTER_PIPELINE> triangle_raster_pipeline;
	std::unique_ptr<RHI_DEPTH_BUFFER> depth_buffer;
	std::unique_ptr<RHI_BUFFER> vertex_buffer;
	std::unique_ptr<RHI_BUFFER> index_buffer;
	std::unique_ptr<RHI_PIPELINE_LAYOUT> pipeline_layout;
	std::unique_ptr<RHI_CONSTANT_BUFFER> camera_shader_buffer;
	std::unique_ptr<RHI_CONSTANT_BUFFER> object_shader_buffer;
	std::unique_ptr<RHI_BUFFER> cpu_camera_shader_buffer;
	std::unique_ptr<RHI_BUFFER> cpu_object_shader_buffer;
	std::vector<RHI_CONSTANT_BUFFER*> constants_buffer_array;
	std::unique_ptr<RHI_COMMAND_QUEUE> copy_command_queue;
	std::unique_ptr<RHI_COMMAND_BUFFER> copy_command_buffer;

	float aspect = 800.0f / 600.0f;
	float x = 0.5f;

	CameraCB camera;
	ObjectCB triangle_transforms;

	get_transforms(triangle_transforms.world, camera.view, camera.projection);

	Transform t;

	t.position = Vec3(0, 0, 0);

	t.scale = Vec3(1, 1, 1);

	t.pivot = Vec3(0.25f, 0.25f, 0.25f);

	RHI_VIEWPORT vp;	
	vp.x = 0;
	vp.y = 0;
	vp.width = 800;
	vp.height = 600;
	vp.min_z = 0.0f;
	vp.max_z = 1.0f;

	struct Vertex
	{
		float x, y, z;
	};
	
	Vertex vertices[] =
	{
		{  0.0f,  x, 0.0f }, // top
		{  x, -x , 0.0f }, // right
		{ -x, -x, 0.0f }  // left
	};
	constexpr unsigned int vertex_count = sizeof(vertices) / sizeof(Vertex);

	uint16_t indices[] =
	{
		0, 1, 2
	};
	constexpr unsigned int index_count = sizeof(indices) / sizeof(uint16_t);

	const std::filesystem::path shaders_folder(R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\dalbora_rt\src\tests)");
	
	test_swap_chain([&](RHI_DEVICE& device, RHI_COMMAND_QUEUE& UNUSED_PARAM(command_queue),
		RHI_COMMAND_BUFFER& UNUSED_PARAM(command_buffer)) {


		std::unique_ptr<RHI_COMPILED_SHADER_BUFFER> vertex_shader;
		std::unique_ptr<RHI_COMPILED_SHADER_BUFFER> pixel_shader;

		// on init

		//if (on_init)
		//	on_init(device, command_queue, command_buffer);

		// create layout for pipeline 
		RHI_PIPELINE_LAYOUT_DESC pl_desc(device);
		pl_desc.shader_type = shader_type_undef;
		
		// add constant buffer descriptors for camera and object transforms;
		RHI_DESCRIPTOR_DESC cb_desc;
		cb_desc.resource_type = resource_type_constant_buffer;
		cb_desc.pool_range_start = 0;
		cb_desc.pool_range_count = 2;
		pl_desc.descriptors.push_back(cb_desc);

		// create pipeline layout
		pipeline_layout = rhi_pipeline_layout_create(pl_desc);

		// compile shaders
		rhi_shaders_compiler_set_folder(shaders_folder.string().c_str());
		vertex_shader = rhi_shaders_compiler_compile("simple_triangle.hlsl", "VSMain", "vs_6_0");
		pixel_shader = rhi_shaders_compiler_compile("simple_triangle.hlsl", "PSMain", "ps_6_0");

		// create geometry buffers
		RHI_VERTEX_BUFFER_DESC vb_desc(device);
		vb_desc.count = vertex_count;
		vb_desc.width = vertex_count * sizeof(Vertex);
		vb_desc.stride = sizeof(Vertex);
		vb_desc.height = 1;
		vb_desc.format = resource_format_float3;
		vertex_buffer = rhi_vertex_buffer_create(vb_desc);

		RHI_INDEX_BUFFER_DESC ib_desc(device);
		ib_desc.count = index_count;
		ib_desc.width = sizeof(uint16_t) * index_count;
		ib_desc.stride = sizeof(uint16_t);
		ib_desc.height = 1;
		ib_desc.format = resource_format_uint16;
		index_buffer = rhi_index_buffer_create(ib_desc);

		// create cpu upload memory for camera transforms
		RHI_BUFFER_DESC cpu_camera_buffer_desc(device);
		cpu_camera_buffer_desc.width  = sizeof(CameraCB);
		cpu_camera_buffer_desc.height = 1;
		cpu_camera_buffer_desc.memory_type = buffer_memory_type_cpu_to_gpu;
		cpu_camera_shader_buffer = rhi_buffers_create_raw(cpu_camera_buffer_desc);

		// create memory for camera transforms
		RHI_BUFFER_DESC camera_buffer_desc(device);
		camera_buffer_desc.width = sizeof(CameraCB);
		camera_buffer_desc.height = 1;
		if (render_api == rhi_api_dx12) {
			DX_BUFFER_DESC dx_buffer_desc;
			dx_buffer_desc.heap_slot = 0;
			camera_buffer_desc.platform_desc_ptr = &dx_buffer_desc;
			camera_shader_buffer = rhi_buffers_create_constant(camera_buffer_desc);
		}
		else {
			camera_shader_buffer = rhi_buffers_create_constant(camera_buffer_desc);
		}

		// create cpu upload memory for object transfrms
		RHI_BUFFER_DESC cpu_object_buffer_desc(device);
		cpu_object_buffer_desc.width = sizeof(ObjectCB);
		cpu_object_buffer_desc.height = 1;
		cpu_object_buffer_desc.memory_type = buffer_memory_type_cpu_to_gpu;
		cpu_object_shader_buffer = rhi_buffers_create_raw(cpu_object_buffer_desc);

		// create memory for object transforms
		RHI_BUFFER_DESC object_buffer_desc(device);
		object_buffer_desc.width = sizeof(ObjectCB);
		object_buffer_desc.height = 1;
		if (render_api == rhi_api_dx12) {
			DX_BUFFER_DESC dx_buffer_desc;
			dx_buffer_desc.heap_slot = 1;
			object_buffer_desc.platform_desc_ptr = &dx_buffer_desc;
			object_shader_buffer = rhi_buffers_create_constant(object_buffer_desc);
		}
		else {
			object_shader_buffer = rhi_buffers_create_constant(object_buffer_desc);
		}		

		constants_buffer_array = { camera_shader_buffer.get(), object_shader_buffer.get() };

		// create copy command queue ad buffer
		RHI_COMMAND_QUEUE_DESC queue_desc(device);
		copy_command_queue = rhi_command_queue_create_for_copy(queue_desc);

		RHI_COMMAND_BUFFER_DESC command_buffer_desc(device, *copy_command_queue);
		copy_command_buffer = rhi_command_buffer_create_for_copy(command_buffer_desc);

		// upload buffers
		rhi_command_queue_execute(*copy_command_queue, true, [&](RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
			std::vector<RHI_COMMAND_BUFFER*>& command_buffer_list) {

				rhi_command_buffer_record(*copy_command_buffer,
					[&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl)) {

						// cpu bridge buffer uploading
						{
							RHI_BUFFER_DESC cpu_buffer_desc(device);
							cpu_buffer_desc.width = vb_desc.width;
							cpu_buffer_desc.stride = vb_desc.stride;
							cpu_buffer_desc.height = vb_desc.height;
							cpu_buffer_desc.memory_type = buffer_memory_type_cpu_to_gpu;
							auto cpu_vertex_buffer = rhi_buffers_create_raw(cpu_buffer_desc);
							rhi_buffers_map_write(*cpu_vertex_buffer, &vertices[0], 0, cpu_buffer_desc.width * cpu_buffer_desc.height);
							rhi_buffers_gpu_upload(*copy_command_buffer, *cpu_vertex_buffer, *vertex_buffer);
						}

						// cpu bridge buffer uploading
						{
							RHI_BUFFER_DESC cpu_buffer_desc(device);
							cpu_buffer_desc.width = ib_desc.width;
							cpu_buffer_desc.stride = ib_desc.stride;
							cpu_buffer_desc.height = ib_desc.height;
							cpu_buffer_desc.memory_type = buffer_memory_type_cpu_to_gpu;
							auto cpu_index_buffer = rhi_buffers_create_raw(cpu_buffer_desc);
							rhi_buffers_map_write(*cpu_index_buffer, &indices[0], 0, cpu_buffer_desc.width * cpu_buffer_desc.height);
							rhi_buffers_gpu_upload(*copy_command_buffer, *cpu_index_buffer, *index_buffer);
						}

					});

				command_buffer_list.push_back(&*copy_command_buffer);
			});

		// create depth buffer
		RHI_DEPTH_BUFFER_DESC db_desc(device);
		db_desc.width = 800;
		db_desc.height = 600;
		db_desc.format = resource_format_d24_norm_s8_uint;
		depth_buffer = rhi_buffers_create_depth(db_desc);

		// create shaders layouts
		std::vector<RHI_INPUT_LAYOUT_DESC> input_layouts;
		input_layouts.emplace_back("POSITION", resource_format_float3, 0);

		// create pipeline
		RHI_RASTER_PIPELINE_DESC pipe_desc(device, *pipeline_layout, input_layouts, vertex_shader.get(), pixel_shader.get());
		pipe_desc.topology = primitive_topology_triangle;
		pipe_desc.surface_format = resource_format_R8G8B8A8_norm;
		pipe_desc.depth_buffer_format = resource_format_d24_norm_s8_uint;
		triangle_raster_pipeline = rhi_raster_pipeline_create(pipe_desc);

		},
		[&](RHI_RENDER_PASS& render_pass) {

			// on before draw
			render_pass.set_depth_buffer(depth_buffer.get());
			render_pass.set_pipeline(triangle_raster_pipeline.get());
			render_pass.set_view_port(vp);
			render_pass.set_constant_buffers(constants_buffer_array);
		},
		[&](RHI_RENDER_PASS& UNUSED_PARAM(render_pass), RHI_COMMAND_BUFFER& command_buffer) {			

			//float dt = get_delta_time();

			//rotate_transform(
			//	t.rotation,
			//	dt,
			//	1.0f,
			//	Vec3(0, 1, 0));

			//triangle_transforms.world = build_matrix(t.position, t.rotation, t.scale, Vec3(0, 0.5, 0));

			// upload shaders constants
			rhi_buffers_map_write(*cpu_camera_shader_buffer, &camera, 0, sizeof(CameraCB));
			rhi_buffers_map_write(*cpu_object_shader_buffer, &triangle_transforms, 0, sizeof(ObjectCB));
			
			rhi_command_queue_execute(*copy_command_queue, true, [&](RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
				std::vector<RHI_COMMAND_BUFFER*>& command_buffer_list) {

					rhi_command_buffer_record(*copy_command_buffer,
						[&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl)) {
							
							rhi_buffers_gpu_upload(*copy_command_buffer, *cpu_camera_shader_buffer, *camera_shader_buffer);
							rhi_buffers_gpu_upload(*copy_command_buffer, *cpu_object_shader_buffer, *object_shader_buffer);

						});
					command_buffer_list.push_back(copy_command_buffer.get());
				});

			// on draw
			rhi_command_buffer_draw_triangle_list(command_buffer, *vertex_buffer, index_buffer.get()); 
		},
		[&](RHI_RENDER_PASS& UNUSED_PARAM(render_pass)) {

			// on after draw

		},
		[&](RHI_DEVICE& UNUSED_PARAM(device)) {

			//if (on_end)
			//	on_end(device);

			// on end
		});
	return;

}

