#ifndef __Mesh_hpp__
#define __Mesh_hpp__

#include "Common.hpp"

class Material;
class Mesh
{
public:
	virtual ~Mesh() = default;
	void setMaterial(std::shared_ptr<Material> material) { mMaterial = material; }
private: 
	std::shared_ptr<Material> mMaterial;
};

#endif
