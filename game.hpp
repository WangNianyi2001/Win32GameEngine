#pragma once

#include "window.hpp"
#include "gameobject.hpp"

namespace Win32GameEngine {
	class Game : public GameObject {
		bool _alive = true;
	protected:
		Window *window;
		virtual void quit() {
			_alive = false;
		}
	public:
		Game() = default;
		bool alive() {
			return _alive;
		}
		virtual void init() override {
			window->event_distributor.add(WM_DESTROY, defaultDestroy);
			window->event_distributor.add(WM_DESTROY, new RawHandler{ [&](HWND, WPARAM, LPARAM) {
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