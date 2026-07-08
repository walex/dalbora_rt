#ifndef __LightNode_hpp__
#define __LightNode_hpp__

#include "SceneNode.hpp"

class LightNode : public SceneNode {
public:
	LightNode(SceneNode* parent) : SceneNode(parent) {}
	virtual ~LightNode() = default;
	size_t mesh_index;
};

#endif // !__LightNode_hpp__
