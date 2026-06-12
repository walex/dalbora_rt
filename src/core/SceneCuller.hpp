#ifndef __SceneCuller_h__
#define __SceneCuller_h__

#include "Common.hpp"
#include "Culler.hpp"

class StaticScene;
class SceneCuller: public Culler
{
public: 
	void setScene(const StaticScene* const scene) { m_scene = scene; }

private: 
	const StaticScene* m_scene;
};

#endif
