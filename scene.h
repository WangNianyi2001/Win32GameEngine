#pragma once

#include "paintable.h"
#include "window.h"
#include <string>
#include <map>
#include <set>

namespace Win32GameEngine {
	using namespace std;

	class Scene {
	public:
		static map<string, Scene *> *all;
		static Scene *active;
		EventHandler event_handler;
		Scene(string name, map<UINT, set<EventHandler::Handler>> handlers = {});
		static bool switchTo(char const *name, HWND hWnd = NULL);
		static LRESULT passEvent(HWND hWnd, UINT type, WPARAM wParam, LPARAM lParam);
	};
}