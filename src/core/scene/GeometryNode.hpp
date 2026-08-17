#ifndef __GeometryNode_hpp__
#define __GeometryNode_hpp__

#include "SceneNode.hpp"

class Mesh;

struct _GeometryInstance
{
	uint vertex_resource_id;
	uint index_resource_id;
	uint material_id;
	float4x4 world;
};

class GeometryNode : public SceneNode {
public:
	GeometryNode(SceneNode* parent, const Mesh& mesh) : SceneNode(parent), m_mesh(mesh) {}
	const Mesh& get_mesh() const { return m_mesh; }
private:
	const Mesh& m_mesh;
};

#endif