#include "Scene.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "gltf_scene.hpp"

void Scene::load(const std::string& scene_path, RhiDevice& device,
	RhiCommandQueue& command_queue) {

	size_t tmp_buffer_offset = 0;
	SCENE_CALLBACKS scene_callbacks;
	RhiCommandBuffer command_buffer;
	RhiSharedBuffer tmp_buffer;

	// create command buffer
	command_buffer.create(device, command_queue);

	// create temmp buffer
	tmp_buffer.create(device, this->m_max_size);

	// scene load callback
	scene_callbacks.on_geometry_loaded = ([&](const RhiDevice& device, RhiCommandBuffer& command_buffer,
		Mesh& mesh, const std::string& attr,
		const uint8_t* const data, const size_t length,
		const size_t stride, const resource_format format) {

			// copy vertices from shared memory to gpu only memory
			if (data != nullptr) {
				tmp_buffer.copy(data, length, tmp_buffer_offset);
				if (attr == "POSITION") {
					mesh.set_vertices(device, command_buffer,
						tmp_buffer, tmp_buffer_offset,
						length, stride, format);
				}
				else if (attr == "NORMAL") {
					mesh.set_normals(device, command_buffer,
						tmp_buffer, tmp_buffer_offset,
						length, stride, format);
				}
				else if (attr == "TEXCOORD_0") {
					mesh.set_uvs(device, command_buffer,
						tmp_buffer, tmp_buffer_offset,
						length, stride, format);
				}
				else if (attr == "__indices__") {

					mesh.set_indices(device, command_buffer,
						tmp_buffer, tmp_buffer_offset,
						length, stride, format);
				}
			}
			tmp_buffer_offset += length;

			if (m_scene_callbacks.on_geometry_loaded)
				m_scene_callbacks.on_geometry_loaded(device, command_buffer,
					mesh, attr,
					data, length,
					stride, format);
	});

	// callbak when have new list of meshes loaded ( AKA 3d model )
	scene_callbacks.on_model_loaded = ([&](const RhiDevice& device, RhiCommandBuffer& command_buffer,
		const std::vector<Mesh*>& meshes) {
		
		if (m_scene_callbacks.on_model_loaded)
			m_scene_callbacks.on_model_loaded(device, command_buffer, meshes);

	});

	// callbak when have new scene node
	scene_callbacks.on_new_scene_node = ([&](const RhiDevice& device, RhiCommandBuffer& command_buffer,
		SceneNode& node) {

		if (m_scene_callbacks.on_new_scene_node)
			m_scene_callbacks.on_new_scene_node(device, command_buffer,
				node);
	});

	scene_callbacks.on_scene_loaded = ([&](const RhiDevice& device, RhiCommandBuffer& command_buffer,
		SceneNode& node) {

			this->m_meshes = std::move(scene_callbacks.meshes);
			this->m_bb_min = scene_callbacks.bb_min;
			this->m_bb_max = scene_callbacks.bb_max;
			this->m_root_node = std::move(scene_callbacks.root_node);

			if (m_scene_callbacks.on_scene_loaded)
				m_scene_callbacks.on_scene_loaded(device, command_buffer,
					node);
	});

	command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {

		command_buffer.record([&] {

			// load scene
			// TODO: check file extension
			load_gltf_scene(device, command_buffer,
				scene_path, 0, scene_callbacks);
		});
		list.add_command_buffer(command_buffer);
	});

	
}


RayTraceScene::RayTraceScene() {

	this->m_rt_buffers.reserve(300);
	this->m_rt_buffers_transforms.reserve(300);

	m_scene_callbacks.on_model_loaded = ([&](const RhiDevice& device, RhiCommandBuffer& command_buffer,
		const std::vector<Mesh*>& meshes) {
	
		// update buffers for ray tracing
		this->new_rt_buffer_for_3d_model(device, command_buffer, meshes);
	});

	m_scene_callbacks.on_new_scene_node = ([&](const RhiDevice& device, RhiCommandBuffer& command_buffer,
		SceneNode& node) {

		// for each child node
		for (auto& child : node.get_childs()) {
			// if found a mesh instance
			if (child->is_leaf()
				&& static_cast<LeafNode*>(child.get())->get_type() == LeafNodeType_MeshGroup) {

				// add rt instance for the mesh
				MeshGroupNode* mesh_node = static_cast<MeshGroupNode*>(child.get());
				this->add_rt_instance_transform(mesh_node->mesh_group_index, mesh_node->get_world_transform());
			}
		}
	});

	m_scene_callbacks.on_scene_loaded = ([&](const RhiDevice& device, RhiCommandBuffer& command_buffer,
		SceneNode& root_node) {

		this->create_rt_instances(device, command_buffer);
	});
}