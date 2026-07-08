#include "Scene.hpp"
#include "Mesh.hpp"
#include "gltf_scene.hpp"

void Scene::load(const std::string& scene_path, RhiDevice& device,
	RhiCommandQueue& command_queue) {

	RhiCommandBuffer command_buffer;

	// create command buffer
	command_buffer.create(device, command_queue);

	// create temmp buffer
	m_tmp_buffer.create(device, this->m_max_size);

	command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {

		command_buffer.record([&] {

			// load scene
			// TODO: check file extension
			load_gltf_scene(device, command_buffer,
				scene_path, 0, this->m_root_node, *this);
		});
		list.add_command_buffer(command_buffer);
	});
	
}

// scene load callback
void Scene::on_geometry_attrib_loaded(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	Mesh& mesh, const std::string& attr,
	const uint8_t* const data, const size_t length,
	const size_t stride, const resource_format format) {

	// copy vertices from shared memory to gpu only memory
	if (data != nullptr) {
		m_tmp_buffer.copy(data, length, tmp_buffer_offset);
		if (attr == "POSITION") {
			mesh.set_vertices(device, command_buffer,
				m_tmp_buffer, tmp_buffer_offset,
				length, stride, format);
		}
		else if (attr == "NORMAL") {
			mesh.set_normals(device, command_buffer,
				m_tmp_buffer, tmp_buffer_offset,
				length, stride, format);
		}
		else if (attr == "TEXCOORD_0") {
			mesh.set_uvs(device, command_buffer,
				m_tmp_buffer, tmp_buffer_offset,
				length, stride, format);
		}
		else if (attr == "__indices__") {

			mesh.set_indices(device, command_buffer,
				m_tmp_buffer, tmp_buffer_offset,
				length, stride, format);
		}
	}
	tmp_buffer_offset += length;
}

void Scene::on_geometry_loaded(std::unique_ptr<Mesh> mesh) {

	this->m_meshes[mesh->get_group_id()].push_back(std::move(mesh));
}

// callbak when have new scene node
void Scene::on_new_scene_node(const RhiDevice& UNUSED_PARAM(device), RhiCommandBuffer& UNUSED_PARAM(command_buffer),
	SceneNode& node) {

	node.update_world_transform();
}

void Scene::on_scene_loaded(const RhiDevice& UNUSED_PARAM(device), RhiCommandBuffer& UNUSED_PARAM(command_buffer),
	const float3 bb_min, const float3 bb_max) {

	m_bb_min = bb_min;
	m_bb_max = bb_max;
}