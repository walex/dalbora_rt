#ifndef _RayTraceScene_hpp__
#define _RayTraceScene_hpp__

#include "Scene.hpp"

class RayTracingRenderer;
class RhiView;
class ResourceManager;
class RayTraceScene : public Scene {
public:
	IMPLEMENT_MOVABLE_CLASS(RayTraceScene);
	RayTraceScene(ResourceManager& rm, const resource_format surface_format);
	virtual ~RayTraceScene() = default;

	void on_geometry_attrib_loaded(RhiCommandBuffer& command_buffer,
		Mesh& mesh, const std::string& name,
		const uint8_t* const data, const size_t length,
		const size_t stride, const resource_format format) override;

	void on_geometry_loaded(std::unique_ptr<Mesh> mesh) override;

	void on_geometry_group_loaded(RhiCommandBuffer& command_buffer, 
		size_t group_id) override;

	void on_new_scene_node(RhiCommandBuffer& command_buffer,
		SceneNode& node) override;

	void on_new_pbr_material(const std::string& name, const PBRMaterialProperties& material_properties) override;

	void on_scene_loaded(RhiCommandBuffer& command_buffer,
		const float3 bb_min, const float3 bb_max) override;

	void load(const std::string& scene_path,
		RhiCommandQueue& command_queue) override;

	void draw(Renderer& render, RhiView& surface_view,
		const RHI_VIEWPORT& viewport);
	
	const RhiRayTracePipeline& get_pipeline() const { return m_pipeline; }
	const RhiShaderBindingTable& get_sbt() const { return m_sbt; }

private:

	void initialize(resource_format surface_format);

	void create_blas_buffer(RhiCommandBuffer& command_buffer,
		const std::vector<std::unique_ptr<Mesh>>& meshes);
	void create_tlas_buffer(RhiCommandBuffer& command_buffer);
	void add_tlas_transform(const size_t model_id, const float4x4& data);

	void setup_shaders();
	void setup_pipeline_layout(resource_format surface_format);
	void setup_pipeline(resource_format surface_format);
private:
	std::vector<RhiRayTraceGeometryBuffer> m_blas_buffers;
	std::vector<std::vector<const float*>> m_tlas_transforms;
	RhiRayTraceGeometrydBufferInstances m_tlas_buffers;
	RhiView m_tlas_view;
	RhiShaderProgram m_ray_gen_shader, m_miss_shader, m_closest_hit_shader;
	RhiRayTracePipeline m_pipeline;
	RhiShaderBindingTable m_sbt;
};


#endif