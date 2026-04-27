#ifndef __Scene_hpp__
#define __Scene_hpp__

#include "Common.hpp"

class Mesh;
class Camera;
class Light;
class Scene;
class Scene
{
public:
	void addGeometry(std::shared_ptr<Mesh> geometry) { mGeometry.push_back(geometry); }
	void addCamera(std::shared_ptr<Camera> camera) { mCamera.push_back(camera); }
	void addLight(std::shared_ptr<Light> light) { mLight.push_back(light); }
private: 
	std::vector<std::shared_ptr<Mesh>> mGeometry;
	std::vector<std::shared_ptr<Camera>> mCamera;
	std::vector<std::shared_ptr<Light>> mLight;

};

#endif
