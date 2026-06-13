#ifndef __SceneCuller_h__
#define __SceneCuller_h__

#include "Common.hpp"
#include "Culler.hpp"

class Scene;
class SceneCuller: public Culler
{
public: 
	void setScene(const Scene* const scene) { m_scene = scene; }

private: 
	const Scene* m_scene;
};

#endif
