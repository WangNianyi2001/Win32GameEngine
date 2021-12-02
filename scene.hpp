#pragma once

#include "gameobject.hpp"
#include <set>

namespace Win32GameEngine {
	using namespace std;
	class Scene : public GameObject {
	public:
		set<Entity> entities;
		Scene() {
			add(GameEventType::UPDATE, [&](GameEvent) {
				for(Entity entity : entities) {
					// TODO
				}
			});
		}
	};
}