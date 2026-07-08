#ifndef __SceneNode_hpp__
#define __SceneNode_hpp__

#include "Spatial.hpp"

class SceneNode : public Spatial {
public:
	IMPLEMENT_COPYABLE_AND_MOVABLE_CLASS(SceneNode)
		SceneNode(SceneNode* parent) : m_parent(parent) {}
	virtual ~SceneNode() = default;
	void add_child(std::unique_ptr<SceneNode> node) { m_childs.push_back(std::move(node)); }
	SceneNode* get_parent() { return m_parent; }
	const std::list<std::unique_ptr<SceneNode>>& get_childs() const { return m_childs; }
	void update_world_transform();
private:
	std::list<std::unique_ptr<SceneNode>> m_childs;
	SceneNode* m_parent = nullptr;
	std::string m_name;
};

#endif