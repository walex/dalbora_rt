#ifndef __Culler_hpp__
#define __Culler_hpp__

#include "Common.hpp"

class Camera;
class Mesh;
class Culler
{
public:
	const std::vector<Mesh*>& getGeometries() { return m_geemetries; }
	const std::vector<Mesh*>& getLights() { return m_geemetries; } // FixME: should be a separate list
	virtual void cull() {}
private: 
	std::vector<Mesh*> m_geemetries;
};

#endif
