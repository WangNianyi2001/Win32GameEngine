#pragma once

#include "window.hpp"
#include "scene.hpp"
#include "gameobject.hpp"

namespace Win32GameEngine {
	class Game : public GameObject {
	protected:
		Window *const window;
		virtual void quit() {
			setactivity(false);
		}
	public:
		Game(Window *const window) : window(window) {}
		virtual void init() override {
			window->events.add(WM_DESTROY, defaultDestroy);
			window->events.add(WM_DESTROY, new RawHandler{ [&](RawEvent) {
				this->quit();
				return 0;
			} });
			window->init();
		}
		virtual void update() override {
			window->update();
		}
	};
}