#ifndef __GeometryNode_hpp__
#define __GeometryNode_hpp__

#include "SceneNode.hpp"

class Mesh;

class GeometryNode : public SceneNode {
public:
	GeometryNode(SceneNode* parent, std::shared_ptr<Mesh> mesh) : SceneNode(parent), m_mesh(mesh) {}
	const std::shared_ptr<Mesh>& get_mesh() const { return m_mesh; }
private:
	std::shared_ptr<Mesh> m_mesh;
};

#endif