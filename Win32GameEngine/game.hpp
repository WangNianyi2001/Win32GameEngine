#pragma once

#include "window.hpp"
#include "scene.hpp"
#include "gameobject.hpp"

namespace Win32GameEngine {
	class Game : public GameObject {
	protected:
		Window *const window;
	public:
		Game(Window *const w) : window(w) {
			add(GameEventType::INIT, [&](GameEvent) {
				window->events.add(WM_QUIT, defaultQuit);
				window->init();
			});
			add(GameEventType::UPDATE, [&](GameEvent) {
				window->update();
			});
			add(GameEventType::KILL, [&](GameEvent) {
				setactivity(false);
			});
		}
		virtual void quit() {
			operator()({ GameEventType::KILL, EventPropagation::DOWN });
		}
	};
}