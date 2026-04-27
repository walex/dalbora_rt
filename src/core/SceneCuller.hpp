#ifndef __SceneCuller_h__
#define __SceneCuller_h__

#include "Common.hpp"
#include "Culler.hpp"

class Scene;
class SceneCuller: public Culler
{
public: 
	void setScene(std::shared_ptr<Scene> scene) { mScene.push_back(scene); }

private: 
	std::vector<std::shared_ptr<Scene>> mScene;
};

#endif
