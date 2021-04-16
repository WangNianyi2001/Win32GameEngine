#pragma once

#pragma warning(disable:4302)
#pragma warning(disable:4244)
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <map>
#include <set>
#include <functional>

namespace Win32GameEngine {
	class Window {
	public:
		struct InitArg {
			LPCWSTR class_name;
			LPCWSTR title;
			UINT class_style = CS_HREDRAW | CS_VREDRAW;
			HICON icon = NULL, icon_small = NULL;
			HCURSOR cursor = LoadCursor(nullptr, IDC_ARROW);
			HBRUSH background_brush = (HBRUSH)(COLOR_WINDOW + 1);
			LPCWSTR menu_name = MAKEINTRESOURCE(IDC_ICON);
			int x = 0, y = 0;
			int width = 640, height = 480;
			DWORD style = WS_OVERLAPPEDWINDOW;
			WNDPROC event_processor;
		};
	private:
		HINSTANCE const hInstance;
		HWND createWindow(InitArg const &args);
		InitArg args;
	public:
		HWND hWnd;
		Window(HINSTANCE hInstance, InitArg const args);
		int run();
	};
	class EventHandler {
	public:
		using Handler = LRESULT(*)(HWND, WPARAM, LPARAM);
	private:
		HWND hWnd;
		static std::map<UINT, std::set<Handler>> handlers;
	public:
		EventHandler() = default;
		LRESULT CALLBACK operator()(HWND hWnd, UINT type, WPARAM wParam, LPARAM lParam);
		void addHandler(UINT type, Handler handler);
		static LRESULT defaultDestroyHandler(HWND, WPARAM, LPARAM);
	};
}