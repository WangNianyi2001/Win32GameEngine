#pragma once

#include "gameobject.hpp"
#include <set>

namespace Win32GameEngine {
	using namespace std;
	class Scene : public GameObject {
	public:
		set<Entity> entities;
		virtual void update() override {
			for(Entity entity : entities) {
				// TODO
			}
		}
	};
}