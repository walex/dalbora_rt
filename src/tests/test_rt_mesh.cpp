#include "test_api.hpp"
#include "gltf_scene.hpp"
#include "RayTraceScene.hpp"
#include "ResourceManager.hpp"

struct alignas(256) _CameraData
{
	float4 camera_pos;
	float4 camera_forward;
	float4 camera_right;
	float4 camera_up;

	float tan_half_fov;
	float aspect;
	float pad0;
	float pad1;
};

static constexpr float image_aspect = 800.0f / 600.0f;

struct alignas(256) GeomrtryInstance {

	uint vertex_resource_id;
	uint index_resource_id;
	uint material_id;
	float4x4 world;
};

void cull_geometries(SceneNode& node, std::vector<GeomrtryInstance>& visible_geometries) {
	GeometryNode* geometry_node = dynamic_cast<GeometryNode*>(&node);
	if (geometry_node != nullptr) {
		GeomrtryInstance& instance = visible_geometries.emplace_back();
		auto& vertex_view = geometry_node->get_mesh().get_vertex_view();
		instance.vertex_resource_id = vertex_view.get_descriptor_id();
		const RhiView* index_view = geometry_node->get_mesh().get_index_view();
		if (index_view == nullptr) {
			throw "Error: GeometryNode has no index buffer";
		}
		instance.index_resource_id = index_view->get_descriptor_id();
		instance.material_id = geometry_node->get_mesh().get_material_id();
		instance.world = geometry_node->get_world_transform();
	}
	else {
		for (auto& child : node.get_childs()) {
			cull_geometries(*child, visible_geometries);
		}
	}
}
void test_rt_mesh_obj(RhiUnitTestCallbacks* callbacks) {

	RhiRenderTarget render_target;
	RhiView render_target_view;
	RhiSharedBuffer camera_transforms;
	RhiView camera_transform_view;
	RhiRayTraceRenderPass rt_render_pass;
	std::unique_ptr<RayTraceScene> scene;
	std::unique_ptr<ResourceManager> resources_manager;
	std::unique_ptr<RhiSharedBufferMap> camera_constant_buffer_map;
	_CameraData camera_matrices;

	RhiUnitTestCallbacks unit_test_callbacks;

	size_t read_only_shaders = 800;
	size_t rw_shaders = 100;
	size_t constant_shaders = 1;

	unit_test_callbacks.on_device_config = ([&](RHI_DEVICE_DESC& device_desc) {
		
		device_desc.features |= device_features_raytracing;
		device_desc.shader_model = hlsl_shader_model_6_8;
	});

	unit_test_callbacks.on_memory_descriptor_config = ([&](size_t& constant_shader_registers_count,
		size_t& read_only_shader_registers_count, size_t& rw_shader_registers_count) {

			return false;
	});

	unit_test_callbacks.on_init = ([&](RhiUnitTest& unit_test) {

		RhiWindow& window = unit_test.window;
		RhiDevice& device = unit_test.device;
		RhiGraphicsCommandQueue& command_queue = unit_test.command_queue;
		RhiSwapChain& swap_chain = unit_test.swap_chain;
		RhiCommandBuffer& command_buffer = unit_test.command_buffer;

		// create resource manager
		resources_manager = std::make_unique<ResourceManager>(device);
		resources_manager->create_descriptor_tables(constant_shaders, read_only_shaders, rw_shaders);

		// create swap chain views
		swap_chain.create_views(device, resources_manager->get_rtv_memory_descriptor());

		// set command buffer memory descriptor
		command_buffer.set_buffers_memory_descriptor(*resources_manager);

		// create scene
		scene = std::make_unique<RayTraceScene>(*resources_manager, swap_chain.get_format());

		// create render target
		render_target.create(device, swap_chain.get_format(),
			window.get_width(), window.get_height());

		if (callbacks)
			callbacks->on_init(unit_test);

		// load scene from file
		std::filesystem::path model_3d_folder = get_executable_folder("test_3d_models");
		std::string model_3d_file = (model_3d_folder / "scene.gltf").string();
		if (std::filesystem::exists(model_3d_file) == false) {
			throw std::exception("3d model file does not exist");
		}
		scene->set_max_size(6 * 1024 * 1024);
		scene->load(model_3d_file, command_queue);

		// create camera and setup transform
		camera_transforms.create(device, sizeof(_CameraData));
		float3 bb_min = scene->get_bb_min();
		float3 bb_max = scene->get_bb_max();

		float4 center = float4((bb_min + bb_max) * 0.5f, 1.0f);

		float3 size =	bb_max - bb_min;

		float max_dimension =
			std::max({
				size.x,
				size.y,
				size.z
				});

		camera_matrices.camera_pos =
			center  + float4(
				0.0f,
				0.0f,
				max_dimension * -0.09f,
				1.0f);
		
		camera_matrices.camera_forward = (center - camera_matrices.camera_pos);
		camera_matrices.camera_forward.xyz = normalize(camera_matrices.camera_forward.xyz);
		camera_matrices.camera_right =
			float4(1.0f, 0.0f, 0.0f, 0.0f);
		camera_matrices.camera_up =
			float4(0.0f, 1.0f, 0.0f, 0.0f);
		camera_matrices.tan_half_fov =
			0.78f;
		camera_matrices.aspect = image_aspect;

		// view render_target (GPU read write)
		render_target_view = render_target.new_view(device, resources_manager->get_rw_buffer_descriptor_slot());

		// add views for transform buffers for shader visibility
		// creation order is related with shader constant buffer registers ids
		camera_transform_view = camera_transforms.new_view(device, shader_view_type_constant_buffer, resources_manager->get_constant_buffer_descriptor_slot());		//  b0

		// create render pass
		rt_render_pass.create(device);

		// map constant buffers
		camera_constant_buffer_map = std::make_unique<RhiSharedBufferMap>(camera_transforms, 0, sizeof(_CameraData));

	});

	unit_test_callbacks.on_draw = ([&](RhiUnitTest& unit_test) {

		// cull geometries
		//std::vector<GeomrtryInstance> visible_geometries;
		//cull_geometries(scene.get_root_node(), visible_geometries);
		//RhiGPUBuffer visible_geometries_buffer;
		//visible_geometries_buffer.create(unit_test.device, visible_geometries.size(), sizeof(GeomrtryInstance), resource_format_none);
		//RhiView visible_geometries_view = visible_geometries_buffer.new_shader_view(unit_test.device);

		rt_render_pass.set_pipeline(scene->get_pipeline());

		// float dt = get_delta_time();

		// upload shaders constants
		memcpy(camera_constant_buffer_map->get_data(), &camera_matrices, sizeof(_CameraData));

		rt_render_pass.set_render_target(render_target_view);

		rt_render_pass.render(unit_test.command_buffer, [&](RhiCommandBuffer& command_buffer) {

			command_buffer.ray_trace(render_target, scene->get_sbt());
		});

		if (callbacks)
			callbacks->on_draw(unit_test);

		unit_test.swap_chain.blit(unit_test.command_buffer, render_target);

		print_fps();
	});

	unit_test_callbacks.on_end = ([&](RhiUnitTest& unit_test) {

		if (callbacks)
			callbacks->on_end(unit_test);

	});

	test_create_swap_chain_obj(&unit_test_callbacks);
}