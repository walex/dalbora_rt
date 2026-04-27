#ifndef __Culler_hpp__
#define __Culler_hpp__

#include "Common.hpp"

class Camera;
class Mesh;
class Culler
{
public:
	const std::vector< std::shared_ptr<Mesh>>& getGeometries() { return mGeometries; }
	const std::vector< std::shared_ptr<Mesh>>& getLights() { return mGeometries; } // FixME: should be a separate list
	virtual void cull() {}
private: 
	std::vector< std::shared_ptr<Mesh>> mGeometries;
};

#endif
