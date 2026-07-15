#ifndef __GeometryNode_hpp__
#define __GeometryNode_hpp__

#include "SceneNode.hpp"

class Mesh;

class GeometryNode : public SceneNode {
public:
	GeometryNode(SceneNode* parent, const Mesh& mesh) : SceneNode(parent), m_mesh(mesh) {}
	const Mesh& get_mesh() const { return m_mesh; }
private:
	const Mesh& m_mesh;
};

#endif