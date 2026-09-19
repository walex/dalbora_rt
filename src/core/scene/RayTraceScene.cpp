#include "RayTraceScene.hpp"
#include "GeometryNode.hpp"
#include "Mesh.hpp"
#include "PBRMaterial.hpp"
#include "RayTracingRenderer.hpp"
#include "ResourceManager.hpp"

static const std::string k_ray_gen_entry_point = "RT_RayGen";
static const std::string k_miss_entry_point = "RT_Miss";
static const std::string k_closest_hit_entry_point = "RT_ClosestHit";

RayTraceScene::RayTraceScene(ResourceManager& rm, const resource_format surface_format)
 : Scene(rm) {

	this->m_blas_buffers.reserve(300);
	this->m_tlas_transforms.reserve(300);

	this->initialize(surface_format);
}

void RayTraceScene::create_blas_buffer(RhiCommandBuffer& command_buffer,
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
	blas_buffer.create(m_resources_manager.get_device(), command_buffer, vertices_ptr, indices_ptr);
}

void RayTraceScene::create_tlas_buffer(RhiCommandBuffer& command_buffer) {

	this->m_tlas_buffers.create(m_resources_manager.get_device(), command_buffer, this->m_blas_buffers, this->m_tlas_transforms);
	this->m_tlas_view = this->m_tlas_buffers.new_view(m_resources_manager.get_device(), m_resources_manager.get_read_only_buffer_descriptor_slot()); // t0
}

void RayTraceScene::add_tlas_transform(const size_t blas_id, const float4x4& data) {

	this->m_tlas_transforms.at(blas_id).push_back(reinterpret_cast<const float*>(&data));
}

void RayTraceScene::on_geometry_attrib_loaded(RhiCommandBuffer& command_buffer,
	Mesh& mesh, const std::string& name,
	const uint8_t* const data, const size_t length,
	const size_t stride, const resource_format format) {

	Scene::on_geometry_attrib_loaded(command_buffer, mesh, name, data, length, stride, format);
}

void RayTraceScene::on_new_scene_node(RhiCommandBuffer& command_buffer,
	SceneNode& node) {

	Scene::on_new_scene_node(command_buffer, node);
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

void RayTraceScene::on_geometry_loaded(std::unique_ptr<Mesh> mesh) {

	// call parent callback to store the mesh in the scene
	Scene::on_geometry_loaded(std::move(mesh));
}

void RayTraceScene::on_geometry_group_loaded(RhiCommandBuffer& command_buffer, 
	size_t group_id) {
	
	this->create_blas_buffer(command_buffer, this->get_meshes(group_id));
}

void RayTraceScene::on_scene_loaded(RhiCommandBuffer& command_buffer,
	const float3 bb_min, const float3 bb_max) {

	this->create_tlas_buffer(command_buffer);
	Scene::on_scene_loaded(command_buffer, bb_min, bb_max);
}


void RayTraceScene::setup_shaders() {

	std::filesystem::path shader_path = get_executable_folder("shaders");
	shader_path = shader_path / "rt_scene.hlsl";
	std::string ray_gen_shader_file = shader_path.string();
	std::string miss_shader_file = shader_path.string();
	std::string closest_hit_shader_file = shader_path.string();

	// compile shaders
	m_ray_gen_shader.create(ray_gen_shader_file,
		k_ray_gen_entry_point, "lib_6_8");
	m_miss_shader.create(miss_shader_file,
		k_miss_entry_point, "lib_6_8");
	m_closest_hit_shader.create(closest_hit_shader_file,
		k_closest_hit_entry_point, "lib_6_8");

}

void RayTraceScene::setup_pipeline_layout(resource_format surface_format) {
	
	// add layout descriptors
	m_pipeline_layout.set_shader_buffers_descriptor_offsets(0, m_resources_manager.get_constant_buffer_descriptor_size(),
		0, m_resources_manager.get_read_only_buffer_descriptor_size(),
		0, m_resources_manager.get_rw_buffer_descriptor_size());

	// create pipeline layout
	m_pipeline_layout.create(m_resources_manager.get_device(), primitive_topology_triangle, surface_format, resource_format_d24_norm_s8_uint);
}

void RayTraceScene::setup_pipeline(resource_format surface_format) {
	
	this->setup_pipeline_layout(surface_format);

	// config ray trace shader
	RhiRayTracePipelineShaderPrograms ray_trace_shader_programs;
	std::vector<RHI_RT_HIT_GROUP_DESC>& hit_groups_desc = ray_trace_shader_programs.hit_groups_desc;
	auto& hg = hit_groups_desc.emplace_back();
	strcpy_s(hg.name_id, "HG_1");
	hg.closest_hit.blob = m_closest_hit_shader;
	strcpy_s(hg.closest_hit.name_id, k_closest_hit_entry_point.c_str());

	std::vector<RHI_RT_SHADER_UNIT_DESC>& miss_shader_desc = ray_trace_shader_programs.miss_shaders_desc;
	auto& miss_1 = miss_shader_desc.emplace_back();
	miss_1.blob = m_miss_shader;
	strcpy_s(miss_1.name_id, k_miss_entry_point.c_str());

	std::vector<RHI_RT_SHADER_UNIT_DESC>& ray_gen_shader_desc = ray_trace_shader_programs.ray_gen_shaders_desc;
	auto& ray_gen = ray_gen_shader_desc.emplace_back();
	strcpy_s(ray_gen.name_id, k_ray_gen_entry_point.c_str());
	ray_gen.blob = m_ray_gen_shader;

	ray_trace_shader_programs.ray_gen_shader = &m_ray_gen_shader;
	ray_trace_shader_programs.miss_shader = &m_miss_shader;
	ray_trace_shader_programs.closest_hit_shader = &m_closest_hit_shader;

	// create pipeline
	m_pipeline.create(m_resources_manager.get_device(), m_pipeline_layout, ray_trace_shader_programs);

	// create shader binding table
	m_sbt.create(m_resources_manager.get_device(), m_pipeline, ray_trace_shader_programs);
}

void RayTraceScene::initialize(resource_format surface_format) {
	
	this->setup_shaders();
	this->setup_pipeline(surface_format);
}

void RayTraceScene::load(const std::string& scene_path,	RhiCommandQueue& command_queue) {

	Scene::load(scene_path, command_queue);
}

void RayTraceScene::draw(Renderer& render, RhiView& surface_view, const RHI_VIEWPORT& viewport) {

	Scene::draw(render, surface_view, viewport);

	render.set_rt_pipeline(m_pipeline);
	render.set_bindig_table(m_sbt);
	render.render(surface_view, viewport);
}