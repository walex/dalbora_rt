#ifndef __Scene_hpp__
#define __Scene_hpp__

#include "Common.hpp"

class Mesh;
class Camera;
class Light;
class StaticScene;
class StaticScene
{
public:
	void addGeometry(const Mesh* const geometry) { m_geometries.push_back(geometry); }
	void addCamera(const Camera* const camera) { m_cameras.push_back(camera); }
	void addLight(const Light* const light) { m_ligths.push_back(light); }
private: 
	std::vector<const Mesh*> m_geometries;
	std::vector<const Camera*> m_cameras;
	std::vector<const Light*> m_ligths;

};

#endif
