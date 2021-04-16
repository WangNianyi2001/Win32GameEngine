#pragma once

#include "paintable.h"
#include "window.h"
#include <map>

namespace Win32GameEngine {
	using namespace std;

	class Scene {
	public:
		EventHandler event_handler;
		Scene(char const *name);
	};
	static map<char const *, Scene *> scenes;
	static Scene *active_scene = nullptr;
	bool switchScene(char const *name, HWND hWnd = NULL);
}