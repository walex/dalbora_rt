#ifndef __SceneCuller_h__
#define __SceneCuller_h__

#include "Common.hpp"
#include "Culler.hpp"

class RayTraceScene;
class SceneCuller: public Culler
{
public: 
	void setScene(const RayTraceScene* const scene) { m_scene = scene; }

private: 
	const RayTraceScene* m_scene;
};

#endif
