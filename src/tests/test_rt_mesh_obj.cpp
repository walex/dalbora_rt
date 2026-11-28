#include "rhi.hpp"
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
void test_rt_mesh_obj() {

	RhiDevice device;
	RHI_DEVICE_DESC device_desc;
	device_desc.adapter_id = -1;
	device_desc.shader_model = hlsl_shader_model_6_8;
	device_desc.app_instance = rhi_get_app_instance();
	device_desc.graphics_queue_count = 1;
	device_desc.compute_queue_count = 1;
	device_desc.copy_queue_count = 1;
	device_desc.features = device_features_raytracing;
	device.create(device_desc);
	// create command allocator pools
	constexpr size_t MAX_COMMAND_ALLOCATORS = 8;
	RhiCommandAllocatorPool::Initialize(device, MAX_COMMAND_ALLOCATORS, queue_type_graphics);
	RhiCommandAllocatorPool::Initialize(device, MAX_COMMAND_ALLOCATORS, queue_type_compute);
	RhiCommandAllocatorPool::Initialize(device, MAX_COMMAND_ALLOCATORS, queue_type_copy);

	std::unique_ptr<ResourceManager> resources_manager;
	std::unique_ptr<RhiMemoryDescriptors> rtv_memory_descriptors;
	{
		RhiWindow window;
		RhiGraphicsCommandQueue command_queue;
		RhiCommandBuffer command_buffer;
		RhiSwapChain swap_chain;
		RhiRenderTarget render_target;
		RhiView render_target_view;
		RhiSharedBuffer camera_transforms;
		RhiView camera_transform_view;
		RhiRayTraceRenderPass rt_render_pass;
		std::unique_ptr<RayTraceScene> scene;
		std::unique_ptr<RhiSharedBufferMap> camera_constant_buffer_map;
		_CameraData camera_matrices;
		

		size_t read_only_shaders = 800;
		size_t rw_shaders = 100;
		size_t constant_shaders = 1;

		RHI_WINDOW_CALLBACKS window_callbacks;
		window_callbacks.on_init = [&](RHI_WINDOW* hwindow) {

			// create resource manager
			resources_manager = std::make_unique<ResourceManager>(device);
			resources_manager->create_descriptor_tables(constant_shaders, read_only_shaders, rw_shaders);
			
			rtv_memory_descriptors = std::make_unique<RhiMemoryDescriptors>(device, memory_descriptor_type_rtv,
				RTV_HEAP_DESCRIPTORS_COUNT);

			command_queue.create(device);
			command_buffer.create(device, command_queue);
			swap_chain.create(window, device, command_queue);
			swap_chain.create_views(device, *rtv_memory_descriptors);

			// set command buffer memory descriptor
			command_buffer.set_buffers_memory_descriptor(*resources_manager);

			// create scene
			scene = std::make_unique<RayTraceScene>(*resources_manager, swap_chain.get_format());

			// create render target
			render_target.create(device, swap_chain.get_format(),
				swap_chain.get_width(), swap_chain.get_height());

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

			float3 size = bb_max - bb_min;

			float max_dimension =
				std::max({
					size.x,
					size.y,
					size.z
					});

			camera_matrices.camera_pos =
				center + float4(
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
			camera_matrices.aspect = static_cast<float>(swap_chain.get_width()) / static_cast<float>(swap_chain.get_height());

			// view render_target (GPU read write)
			render_target_view = render_target.new_view(device, resources_manager->get_rw_buffer_descriptor_slot());

			// add views for transform buffers for shader visibility
			// creation order is related with shader constant buffer registers ids
			camera_transform_view = camera_transforms.new_view(device, shader_view_type_constant_buffer, resources_manager->get_constant_buffer_descriptor_slot());		//  b0

			// create render pass
			rt_render_pass.create(device);

			// map constant buffers
			camera_constant_buffer_map = std::make_unique<RhiSharedBufferMap>(camera_transforms, 0, sizeof(_CameraData));

		};

		window_callbacks.on_idle = [&](RHI_WINDOW* hwindow) {

			rt_render_pass.set_pipeline(scene->get_pipeline());

			// upload shaders constants
			memcpy(camera_constant_buffer_map->get_data(), &camera_matrices, sizeof(_CameraData));

			rt_render_pass.set_render_target(render_target_view);
			swap_chain.get_next_render_target();

			command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {
				command_buffer.record([&] {
					rt_render_pass.render(command_buffer, [&](RhiCommandBuffer& command_buffer) {

						command_buffer.ray_trace(render_target, scene->get_sbt());
						swap_chain.blit(command_buffer, render_target);
					});
				});
				list.add_command_buffer(command_buffer);
			});


			
			swap_chain.present();
			print_fps();
		};

		window_callbacks.on_end = [&](RHI_WINDOW* hwindow) {

			
		};

		window.create("test", 800, 600, false, window_callbacks);
		window.mainLoop();
	}
	RhiCommandAllocatorPool::Terminate(queue_type_graphics);
	RhiCommandAllocatorPool::Terminate(queue_type_compute);
	RhiCommandAllocatorPool::Terminate(queue_type_copy);
}