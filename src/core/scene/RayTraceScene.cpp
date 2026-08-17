#include "RayTraceScene.hpp"
#include "GeometryNode.hpp"
#include "Mesh.hpp"
#include "PBRMaterial.hpp"

RayTraceScene::RayTraceScene() {

	this->m_blas_buffers.reserve(300);
	this->m_tlas_transforms.reserve(300);
}

void RayTraceScene::create_blas_buffer(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	const std::vector<std::unique_ptr<Mesh>>& meshes) {

	std::vector<RHI_BUFFER*> vertices_ptr;
	std::vector<RHI_BUFFER*> indices_ptr;
	vertices_ptr.reserve(meshes.size());
	indices_ptr.reserve(meshes.size());
	for (auto& mesh : meshes) {
		vertices_ptr.push_back(mesh->get_vertex_buffer());
		RhiBuffer* index_buffer = mesh->get_index_buffer();
		if (index_buffer != nullptr)
			indices_ptr.push_back(*index_buffer);
		else
			indices_ptr.push_back(nullptr);
	}
	auto& blas_buffer = this->m_blas_buffers.emplace_back();
	this->m_tlas_transforms.emplace_back();
	blas_buffer.create(device, command_buffer, vertices_ptr, indices_ptr);
}

void RayTraceScene::create_tlas_buffer(const RhiDevice& device, RhiCommandBuffer& command_buffer) {

	this->m_tlas_buffers.create(device, command_buffer, this->m_blas_buffers, this->m_tlas_transforms);
	this->m_tlas_view = this->m_tlas_buffers.new_view(device); // t0
}

void RayTraceScene::add_tlas_transform(const size_t blas_id, const float4x4& data) {

	this->m_tlas_transforms.at(blas_id).push_back(reinterpret_cast<const float*>(&data));
}

void RayTraceScene::on_geometry_attrib_loaded(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	Mesh& mesh, const std::string& name,
	const uint8_t* const data, const size_t length,
	const size_t stride, const resource_format format) {

	Scene::on_geometry_attrib_loaded(device, command_buffer, mesh, name, data, length, stride, format);
}

void RayTraceScene::on_new_scene_node(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	SceneNode& node) {

	Scene::on_new_scene_node(device, command_buffer, node);
	GeometryNode* geometry_node = dynamic_cast<GeometryNode*>(&node);
	if (geometry_node != nullptr) {
		this->add_tlas_transform(geometry_node->get_mesh().get_group_id(), geometry_node->get_world_transform());
	}
}

void RayTraceScene::on_new_pbr_material(const std::string& name, const PBRMaterialProperties& material_properties) {

	std::unique_ptr<PBRMaterial> material = std::make_unique<PBRMaterial>(name);
	material->set_properties(material_properties);
	m_materials.push_back(std::move(material));
}

void RayTraceScene::on_geometry_loaded(const RhiDevice& device, std::unique_ptr<Mesh> mesh) {

	// call parent callback to store the mesh in the scene
	Scene::on_geometry_loaded(device, std::move(mesh));
}

void RayTraceScene::on_geometry_group_loaded(const RhiDevice& device, RhiCommandBuffer& command_buffer, 
	size_t group_id) {
	
	this->create_blas_buffer(device, command_buffer, this->get_meshes(group_id));
}

void RayTraceScene::on_scene_loaded(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	const float3 bb_min, const float3 bb_max) {

	this->create_tlas_buffer(device, command_buffer);
	Scene::on_scene_loaded(device, command_buffer, bb_min, bb_max);
}

void RayTraceScene::load(const std::string& scene_path, RhiDevice& device,
	RhiCommandQueue& command_queue) {

	Scene::load(scene_path, device, command_queue);

}