#include "SceneNode.hpp"

void SceneNode::update_world_transform()
{
    if (m_is_local_transform_dirty == false)
        return;

    SceneNode* parent = this->get_parent();
    if (parent)
    {
        m_parent->update_world_transform();
        m_world_transform = parent->get_world_transform() * m_local_transform;
    }
    else
    {
        m_world_transform = m_local_transform;
    }
	m_is_local_transform_dirty = false;
}