#include "RayTraceScene.hpp"
#include "GeometryNode.hpp"
#include "Mesh.hpp"
#include "PBRMaterial.hpp"
#include "RayTracingRenderer.hpp"


static const std::string k_ray_gen_entry_point = "RT_RayGen";
static const std::string k_miss_entry_point = "RT_Miss";
static const std::string k_closest_hit_entry_point = "RT_ClosestHit";

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

void RayTraceScene::setup_pipeline_layout(RhiDevice& device, resource_format surface_format,
	size_t read_only_shader_registers_count, size_t rw_shader_registers_count, 
	size_t constant_shader_registers_count) {
	// add layout descriptors ( order mathers )

	// 1 - GPU read only shader registers range to be used (Scene BVH, geometries, materials, lights, cameras)
	m_pipeline_layout.add_read_only_buffer_descriptors(0, read_only_shader_registers_count,0);

	// 2 - GPU read write shader registers range to be used (Render buffer)
	m_pipeline_layout.add_rw_buffer_descriptors(0, rw_shader_registers_count,0);

	// 3 - Constant buffer shader registers range to be used (Camera matrix)
	m_pipeline_layout.add_constants_buffer_descriptors(0, constant_shader_registers_count,0);

	// create pipeline layout
	m_pipeline_layout.create(device, primitive_topology_triangle, surface_format, resource_format_d24_norm_s8_uint);
}

void RayTraceScene::setup_pipeline(RhiDevice& device, resource_format surface_format,
	size_t read_only_shader_registers_count, size_t rw_shader_registers_count,
	size_t constant_shader_registers_count) {
	
	this->setup_pipeline_layout(device, surface_format, read_only_shader_registers_count,
		rw_shader_registers_count, constant_shader_registers_count);

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
	m_pipeline.create(device, m_pipeline_layout, ray_trace_shader_programs);

	// create shader binding table
	m_sbt.create(device, m_pipeline, ray_trace_shader_programs);
}

void RayTraceScene::initialize(RhiDevice& device, resource_format surface_format,
	size_t read_only_shader_registers_count, size_t rw_shader_registers_count,
	size_t constant_shader_registers_count) {
	
	this->setup_shaders();
	this->setup_pipeline(device, surface_format, read_only_shader_registers_count,
		rw_shader_registers_count, constant_shader_registers_count);
}

void RayTraceScene::load(const std::string& scene_path, RhiDevice& device,
	RhiCommandQueue& command_queue) {

	Scene::load(scene_path, device, command_queue);
}

void RayTraceScene::draw_scene(Renderer& render, RhiView& surface_view, const RHI_VIEWPORT& viewport) {

	Scene::draw_scene(render, surface_view, viewport);

	render.set_rt_pipeline(m_pipeline);
	render.set_bindig_table(m_sbt);
	render.draw(surface_view, viewport);
}