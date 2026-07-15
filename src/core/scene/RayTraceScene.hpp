#ifndef _RayTraceScene_hpp__
#define _RayTraceScene_hpp__

#include "Scene.hpp"

class RayTraceScene : public Scene {
public:
	IMPLEMENT_COPYABLE_AND_MOVABLE_CLASS(RayTraceScene);
	RayTraceScene();
	virtual ~RayTraceScene() = default;

	void on_geometry_attrib_loaded(const RhiDevice& device, RhiCommandBuffer& command_buffer,
		Mesh& mesh, const std::string& name,
		const uint8_t* const data, const size_t length,
		const size_t stride, const resource_format format) override;

	void on_geometry_loaded(std::unique_ptr<Mesh> mesh) override;

	void on_geometry_group_loaded(const RhiDevice& device, RhiCommandBuffer& command_buffer, 
		size_t group_id) override;

	void on_new_scene_node(const RhiDevice& device, RhiCommandBuffer& command_buffer,
		SceneNode& node) override;

	void on_new_pbr_material(const std::string& name, const PBRMaterialProperties& material_properties) override;

	void on_scene_loaded(const RhiDevice& device, RhiCommandBuffer& command_buffer,
		const float3 bb_min, const float3 bb_max) override;


private:
	void create_blas_buffer(const RhiDevice& device, RhiCommandBuffer& command_buffer,
		const std::vector<std::unique_ptr<Mesh>>& meshes);
	void create_tlas_buffer(const RhiDevice& device, RhiCommandBuffer& command_buffer);
	void add_tlas_transform(const size_t model_id, const float4x4& data);
private:
	std::vector<RhiRayTraceGeometryBuffer> m_blas_buffers;
	std::vector<std::vector<const float*>> m_tlas_transforms;
	RhiRayTraceGeometrydBufferInstances m_tlas_buffers;
	RhiView m_tlas_view;
};


#endif