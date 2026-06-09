#include "test_api.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "../../../tinygltf/tiny_gltf.h"

void model_3d_from_file(const std::string& filename, tinygltf::Model& model) {

	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
	//bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename); // for binary glTF(.glb)

	if (!warn.empty()) {
		printf("Warn: %s\n", warn.c_str());
	}

	if (!err.empty()) {
		printf("Err: %s\n", err.c_str());
	}

	if (!ret) {
		printf("Failed to parse glTF: %s\n", filename.c_str());
	}
}


static constexpr float aspect = 800.0f / 600.0f;

static struct Mesh {

	RhiGPUBuffer vb;
	std::unique_ptr<RhiGPUBuffer> ib;	

	RhiSharedBuffer vb_shared;
	RhiSharedBuffer ib_shared;
};

static struct Spatial {

	Eigen::Matrix4f transform;
};

static struct ModelNode : public Spatial {
	std::string name;
};

static struct Model {
	std::list<Mesh*> meshes;
	std::vector<RHI_BUFFER*> vertices_ptr;
	std::vector<RHI_BUFFER*> indices_ptr;
	std::list<ModelNode> nodes;
};

static struct Scene {
	std::list<std::shared_ptr<Model>> models;
	std::list<Mesh> meshes;
	Eigen::Vector3f bb_min, bb_max;
};

resource_format gltfFormatToDxgiFormat(
	int componentType,
	int type)
{
	if (componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
	{
		switch (type)
		{
		case TINYGLTF_TYPE_SCALAR:
			return resource_format_float;

		case TINYGLTF_TYPE_VEC2:
			return resource_format_float2;

		case TINYGLTF_TYPE_VEC3:
			return resource_format_float3;

		case TINYGLTF_TYPE_VEC4:
			return resource_format_float4;
		}
	}
	else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
		return resource_format_uint16;
	}
	else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
		return resource_format_uint32;
	}

	return resource_format_none;
}

void compute_scene_bounds(
	const tinygltf::Model& model,
	const Eigen::Matrix4f& world_matrix,
	Eigen::Vector3f& scene_min,
	Eigen::Vector3f& scene_max)
{
	scene_min =
	{
		FLT_MAX,
		FLT_MAX,
		FLT_MAX
	};

	scene_max =
	{
		-FLT_MAX,
		-FLT_MAX,
		-FLT_MAX
	};

	for (const auto& mesh : model.meshes)
	{
		for (const auto& primitive : mesh.primitives)
		{
			auto pos_it = primitive.attributes.find("POSITION");

			if (pos_it == primitive.attributes.end())
				continue;

			const tinygltf::Accessor& accessor =
				model.accessors[pos_it->second];

			if (accessor.minValues.size() < 3 ||
				accessor.maxValues.size() < 3)
			{
				continue;
			}

			Eigen::Vector3f local_min(
				static_cast<float>(accessor.minValues[0]),
				static_cast<float>(accessor.minValues[1]),
				static_cast<float>(accessor.minValues[2]));

			Eigen::Vector3f local_max(
				static_cast<float>(accessor.maxValues[0]),
				static_cast<float>(accessor.maxValues[1]),
				static_cast<float>(accessor.maxValues[2]));

			Eigen::Vector3f corners[8] =
			{
				{local_min.x(), local_min.y(), local_min.z()},
				{local_max.x(), local_min.y(), local_min.z()},
				{local_min.x(), local_max.y(), local_min.z()},
				{local_max.x(), local_max.y(), local_min.z()},
				{local_min.x(), local_min.y(), local_max.z()},
				{local_max.x(), local_min.y(), local_max.z()},
				{local_min.x(), local_max.y(), local_max.z()},
				{local_max.x(), local_max.y(), local_max.z()}
			};

			for (int i = 0; i < 8; ++i)
			{
				Eigen::Vector4f p(
					corners[i].x(),
					corners[i].y(),
					corners[i].z(),
					1.0f);

				p = world_matrix * p;

				scene_min =
					scene_min.cwiseMin(p.head<3>());

				scene_max =
					scene_max.cwiseMax(p.head<3>());
			}
		}
	}
}

void load_gltf_scene(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	const std::string& file_path, Scene& scene ) {

	tinygltf::Model gltf_model;
	model_3d_from_file(file_path, gltf_model);	
	int hh = 0;
	std::map<size_t, std::shared_ptr<Model>> node_models;
	for (size_t i = 0; i < gltf_model.meshes.size(); i++)
	{
		const auto& gltf_mesh = gltf_model.meshes.at(i);	
		auto& node_model = node_models[i] = std::make_shared<Model>();
		for (auto& gltf_primitive : gltf_mesh.primitives) {

			Mesh& mesh = scene.meshes.emplace_back();
			node_model->meshes.emplace_back(&mesh);

			// POSITION
			int positionAccessorIndex =
				gltf_primitive.attributes.at("POSITION");

			const auto& positionAccessor =
				gltf_model.accessors[positionAccessorIndex];

			const auto& positionBufferView =
				gltf_model.bufferViews[positionAccessor.bufferView];

			const auto& positionBuffer =
				gltf_model.buffers[positionBufferView.buffer];

			const uint8_t* positions =
				positionBuffer.data.data()
				+ positionBufferView.byteOffset
				+ positionAccessor.byteOffset;

			size_t vertexSize =
				positionAccessor.ByteStride(positionBufferView);

			size_t vertexBytes =
				positionAccessor.count * vertexSize;

			mesh.vb.create(device, vertexBytes, vertexSize, gltfFormatToDxgiFormat(
				positionAccessor.componentType,
				positionAccessor.type));

			mesh.vb_shared.create(device, vertexBytes);
			// copy vertices from cpu visible memory to gpu
			auto v_map_info = mesh.vb_shared.map(0, vertexBytes);
			memcpy(v_map_info.get_data(), positions, v_map_info.get_length());
			mesh.vb_shared.unmap(v_map_info);
			mesh.vb.upload(command_buffer, mesh.vb_shared);
			node_model.get()->vertices_ptr.emplace_back(mesh.vb);

			if (gltf_primitive.indices >= 0) {
				mesh.ib = std::make_unique<RhiGPUBuffer>();

				const auto& indexAccessor =
					gltf_model.accessors[gltf_primitive.indices];

				const auto& indexBufferView =
					gltf_model.bufferViews[indexAccessor.bufferView];

				const auto& indexBuffer =
					gltf_model.buffers[indexBufferView.buffer];

				size_t indexSize = 0;
				switch (indexAccessor.componentType)
				{
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
					indexSize = sizeof(uint8_t);
					break;

				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
					indexSize = sizeof(uint16_t);
					break;

				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
					indexSize = sizeof(uint32_t);
					break;

				default:
					throw std::runtime_error("Unsupported index format");
				}

				size_t indexBytes =
					indexAccessor.count * indexSize;

				const uint8_t* indices =
					indexBuffer.data.data()
					+ indexBufferView.byteOffset
					+ indexAccessor.byteOffset;

				mesh.ib->create(device, indexBytes, indexSize, gltfFormatToDxgiFormat(
					indexAccessor.componentType,
					indexAccessor.type));
				mesh.ib_shared.create(device, indexBytes);

				size_t offset =
					indexBufferView.byteOffset +
					indexAccessor.byteOffset;

				// copy indices from cpu visible memory to gpu
				auto i_map_info = mesh.ib_shared.map(0, indexBytes);
				memcpy(i_map_info.get_data(), indices, i_map_info.get_length());
				mesh.ib_shared.unmap(i_map_info);
				mesh.ib->upload(command_buffer, mesh.ib_shared);
				node_model.get()->indices_ptr.push_back(*mesh.ib);
			}
			else {
				node_model.get()->indices_ptr.push_back(nullptr);
			}
		}
	}

	for (auto& gltf_node : gltf_model.nodes) {

		if (gltf_node.mesh >= 0) {
			auto& model = node_models[gltf_node.mesh];
			scene.models.push_back(model);
			ModelNode& node = model->nodes.emplace_back();

			Eigen::Matrix4f M = Eigen::Matrix4f::Identity();
			if (!gltf_node.translation.empty())
			{
				Eigen::Matrix4f T = Eigen::Matrix4f::Identity();

				T(0, 3) = static_cast<float>(gltf_node.translation[0]);
				T(1, 3) = static_cast<float>(gltf_node.translation[1]);
				T(2, 3) = static_cast<float>(gltf_node.translation[2]);

				M *= T;
			}

			if (!gltf_node.rotation.empty())
			{
				Eigen::Quaternionf Q(
					gltf_node.rotation[3], // w
					gltf_node.rotation[0], // x
					gltf_node.rotation[1], // y
					gltf_node.rotation[2]); // z

				Eigen::Matrix4f R = Eigen::Matrix4f::Identity();

				R.block<3, 3>(0, 0) = Q.toRotationMatrix();


				M *= R;
			}

			if (!gltf_node.scale.empty())
			{
				Eigen::Matrix4f S = Eigen::Matrix4f::Identity();
				S.block<3, 3>(0, 0) =
					Eigen::Scaling(
						(float)gltf_node.scale[0],
						(float)gltf_node.scale[1],
						(float)gltf_node.scale[2]);

				M *= S;
			}
			node.transform = M;

		}
	}

	compute_scene_bounds(gltf_model, Eigen::Matrix4f::Identity(), scene.bb_min, scene.bb_max);
}

void test_rt_mesh_obj(RhiUnitTestCallbacks* callbacks) {	


	RhiRenderTarget render_target;
	RhiView render_target_view;
	RhiShaderProgram ray_gen_shader;
	RhiShaderProgram miss_shader;
	RhiShaderProgram closest_hit_shader;
	RhiGPUBuffer vertex_buffer;
	RhiGPUBuffer index_buffer;
	RhiSharedBuffer camera_transforms;
	RhiView camera_transform_view;
	std::vector<RhiRayTraceGeometryBuffer> geometry_buffers;
	RhiRayTraceGeometrydBufferInstances geometry_instances;
	RhiView geometry_instances_views;
	RhiRayTraceRenderPass rt_render_pass;
	RhiShaderBindingTable sbt;
	Scene scene;

	std::unique_ptr<RhiSharedBufferMap> camera_constant_buffer_map;
	CameraCBRT camera_matrices;

	RhiUnitTestCallbacks unit_test_callbacks;
	unit_test_callbacks.on_init = ([&](RhiUnitTest& unit_test) {

		RhiWindow& window = unit_test.window;
		RhiDevice& device = unit_test.device;
		RhiGraphicsCommandQueue& command_queue = unit_test.command_queue;
		RhiCommandBuffer& command_buffer = unit_test.command_buffer;
		RhiSwapChain& swap_chain = unit_test.swap_chain;
		RhiPipelineLayout& pipeline_layout = unit_test.pipeline_layout;
		RhiRayTracePipeline& pipeline = unit_test.ray_trace_pipeline;
		
		// setup shaders
		unit_test.ray_gen_shader_file = R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\dalbora_rt\src\tests\simple_rt.hlsl)";
		unit_test.miss_shader_file = R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\dalbora_rt\src\tests\simple_rt.hlsl)";
		unit_test.closest_hit_shader_file = R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\dalbora_rt\src\tests\simple_rt.hlsl)";

		// create render target
		render_target.create(device, swap_chain.get_format(),
			window.get_width(), window.get_height());

		if (callbacks)
			callbacks->on_init(unit_test);

		// compile shaders
		ray_gen_shader.create(unit_test.ray_gen_shader_file,
			"RayGen", "lib_6_6");
		miss_shader.create(unit_test.miss_shader_file,
			"Miss", "lib_6_6");
		closest_hit_shader.create(unit_test.closest_hit_shader_file,
			"ClosestHit", "lib_6_6");

		// add layout descriptors ( order mathers )

		// 1 - GPU read only (Scene BVH)
		pipeline_layout.add_read_only_buffer_descriptors(0, 100);

		// 2 - GPU read write (Render Target)
		pipeline_layout.add_rw_buffer_descriptors(0, 100);

		// 3 - Constant buffer (Camera)
		pipeline_layout.add_constants_buffer_descriptors(0, 100);		

		// create pipeline layout
		pipeline_layout.create(device, primitive_topology_triangle, swap_chain.get_format(), resource_format_d24_norm_s8_uint);

		// rt pipeline config
		std::string ray_gen_entry_point = "RayGen";
		std::string miss_entry_point = "Miss";
		std::string closest_hit_entry_point = "ClosestHit";

		// config ray trace shader
		RhiRayTracePipelineShaderPrograms ray_trace_shader_programs;
		std::vector<RHI_RT_HIT_GROUP_DESC>& hit_groups_desc = ray_trace_shader_programs.hit_groups_desc;
		auto& hg = hit_groups_desc.emplace_back();
		strcpy_s(hg.name_id, "HG_1");
		hg.closest_hit.blob = closest_hit_shader;
		strcpy_s(hg.closest_hit.name_id, closest_hit_entry_point.c_str());		

		std::vector<RHI_RT_SHADER_UNIT_DESC>& miss_shader_desc = ray_trace_shader_programs.miss_shaders_desc;
		auto& miss_1 = miss_shader_desc.emplace_back();
		miss_1.blob = miss_shader;
		strcpy_s(miss_1.name_id, miss_entry_point.c_str());
		
		std::vector<RHI_RT_SHADER_UNIT_DESC>& ray_gen_shader_desc = ray_trace_shader_programs.ray_gen_shaders_desc;
		auto& ray_gen = ray_gen_shader_desc.emplace_back();
		strcpy(ray_gen.name_id, ray_gen_entry_point.c_str());
		ray_gen.blob = ray_gen_shader;

		ray_trace_shader_programs.ray_gen_shader = &ray_gen_shader;
		ray_trace_shader_programs.miss_shader = &miss_shader;
		ray_trace_shader_programs.closest_hit_shader = &closest_hit_shader;

		pipeline.create(device, pipeline_layout, ray_trace_shader_programs);

		// shader binding table
		sbt.create(device, pipeline, ray_trace_shader_programs);

		// create camera transform buffer
		camera_transforms.create(device, sizeof(CameraCBRT));		

		// upload vertices e indices data to gpu only memory
		command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {

			command_buffer.record([&] {

				// load scene
				load_gltf_scene(device,
					command_buffer,
					//R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\models_3d\InteriorTest.obj.gltf)",
					//R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\models_3d\FinalBaseMesh.gltf)",
					R"(C:\Users\wadrw\Documents\develop\projects\personal\rtx\models_3d\SheenChair.gltf)",
					scene);

				Eigen::Vector3f center =
					(scene.bb_min + scene.bb_max) * 0.5f;

				Eigen::Vector3f size =
					scene.bb_max - scene.bb_min;

				float max_dimension =
					std::max({
						size.x(),
						size.y(),
						size.z()
						});

				camera_matrices.camera_pos =
					center + Eigen::Vector3f(
						0.0f,
						0.0f,
						max_dimension * 1.0f);
				camera_matrices.camera_forward = (center - camera_matrices.camera_pos).normalized();
				camera_matrices.camera_right =
					Vec3(1.0f, 0.0f, 0.0f);
				camera_matrices.camera_up =
					Vec3(0.0f, 1.0f, 0.0f);
				camera_matrices.tanHalfFov =
					0.7002075f;
				camera_matrices.aspect = aspect;

				std::vector<std::vector<float*>> instances_transforms;
				for (auto& model : scene.models) {

					std::vector<float*>& transforms = instances_transforms.emplace_back();;
					for (auto& node : model.get()->nodes) {

						transforms.push_back(node.transform.data());
					}
					auto& geometry_buffer = geometry_buffers.emplace_back();
					geometry_buffer.create(device, command_buffer, model.get()->vertices_ptr, model.get()->indices_ptr);
				}
				geometry_instances.create(device, command_buffer, geometry_buffers, instances_transforms);
				geometry_instances_views = geometry_instances.new_view(device);
			});

			list.add_command_buffer(command_buffer);
		});

		// view render_target (GPU read write)
		render_target_view = render_target.new_rw_view(device);

		// view camera (constant buffer)
		camera_transform_view = camera_transforms.new_constant_buffer_view(device);		// cb reg 0

		// create render pass
		rt_render_pass.create(device);
		
		// add views for transform buffers for shader visibility
		// creation order is related with shader constant buffer registers ids
		camera_transform_view = camera_transforms.new_constant_buffer_view(device);		// cb reg 0
		
		// map constant buffers
		camera_constant_buffer_map = std::make_unique<RhiSharedBufferMap>(camera_transforms.map(0, sizeof(CameraCB)));
		
	});

	unit_test_callbacks.on_draw = ([&](RhiUnitTest& unit_test) {

		rt_render_pass.set_pipeline(unit_test.ray_trace_pipeline);

		float dt = get_delta_time();
		//rotation_matrix = rotate_triangle(dt);
	//	geometry_buffer.update(unit_test.device, unit_test.command_buffer, geometry_instances);

		// upload shaders constants
		memcpy(camera_constant_buffer_map->get_data(), &camera_matrices, sizeof(CameraCB));
		
		rt_render_pass.set_render_target(render_target_view);

		rt_render_pass.render(unit_test.command_buffer, [&](RhiCommandBuffer& command_buffer) {

			command_buffer.ray_trace(render_target, sbt);
		});

		if (callbacks)
			callbacks->on_draw(unit_test);

		unit_test.swap_chain.blit(unit_test.command_buffer, render_target);

	});

	unit_test_callbacks.on_end = ([&](RhiUnitTest& unit_test) {

		if (callbacks)
			callbacks->on_end(unit_test);
		
		camera_transforms.unmap(*camera_constant_buffer_map);
	});

	test_create_swap_chain_obj(&unit_test_callbacks);
}