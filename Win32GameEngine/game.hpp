#pragma once

#include "window.hpp"
#include "scene.hpp"
#include "gameobject.hpp"

namespace Win32GameEngine {
	class Game : public GameObject<void, Scene> {
	protected:
		Window *const window;
	public:
		Game(Window *const w) : window(w) {
			add(GameEventType::INIT, [&](GameEvent) {
				window->events.add(WM_SYSCOMMAND, [&](SystemEvent event) {
					switch(event.data.wParam) {
					case SC_RESTORE:
						// Fail to restore minimized window
						window->restore();
						break;
					case SC_MINIMIZE:
						return 0;
					default:
						event.defaultBehavior();
					}
					return 0;
				});
				window->events.add(WM_QUIT, defaultQuit);
				window->init();
			});
			add(GameEventType::UPDATE, [&](GameEvent) {
				window->update();
			});
			add(GameEventType::KILL, [&](GameEvent) {
				inactivate();
			});
		}
		virtual void quit() {
			operator()({ GameEventType::KILL, EventPropagation::DOWN });
		}
	};
}