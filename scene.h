#pragma once

#include "paintable.h"
#include "window.h"
#include <map>
#include <set>

namespace Win32GameEngine {
	using namespace std;

	class Scene {
	public:
		static map<char const *, Scene *> *all;
		static Scene *active;
		void (*init)(HWND, void *);
		EventHandler event_handler;
		Scene(char const *name, void (*init)(HWND, void *), map<UINT, set<EventHandler::Handler>> handlers = {});
		static LRESULT passEvent(HWND hWnd, UINT type, WPARAM wParam, LPARAM lParam);
		static bool switchTo(char const *name, HWND hWnd, void *args);
	};
}