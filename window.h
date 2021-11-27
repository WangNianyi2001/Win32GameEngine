#pragma once

#pragma warning(disable:4302)
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <map>
#include <set>
#include <functional>

namespace Win32GameEngine {
	using namespace std;

	template<typename T>
	struct UltraCompare {
		bool operator()(const T &a, const T &b) const {
			auto pa = (unsigned char *)&a, pb = (unsigned char *)&b;
			for(unsigned l = sizeof(T); l--; ) {
				if(*pa < *pb)
					return true;
			}
			return false;
		}
	};

	struct EventDistributor {
		using Handler = function<LRESULT(HWND, WPARAM, LPARAM)>;
		using TypedHandlers = set<Handler, UltraCompare<Handler>>;
		using HandlerMap = map<UINT, TypedHandlers>;
		HandlerMap handler_map;
		EventDistributor() = default;
		EventDistributor(HandlerMap handlers);
		LRESULT CALLBACK operator()(HWND hWnd, UINT type, WPARAM wParam, LPARAM lParam);
		void add(UINT type, Handler handler);
	};

	class Window {
		static LRESULT CALLBACK event_processor(HWND, UINT, WPARAM, LPARAM);
		using HWndMap = map<HWND, Window *>;
		static HWndMap hwnd_map;
	private:
		HWND hWnd;
	public:
		EventDistributor event_distributor;
		struct InitArg {
			LPCWSTR class_name;
			HINSTANCE instance = nullptr;
			LPCWSTR title = nullptr;
			UINT class_style = CS_HREDRAW | CS_VREDRAW;
			HICON icon = nullptr, icon_small = nullptr;
			HCURSOR cursor = nullptr;
			HBRUSH background_brush = nullptr;
			LPCWSTR menu_name = nullptr;
			int x = 0, y = 0;
			int width = 640, height = 480;
			DWORD style = WS_OVERLAPPEDWINDOW;
		};
		Window(InitArg const init_args);
		bool ready();
		WPARAM activate();
	};
	
	LRESULT defaultDestroyHandler(HWND, WPARAM, LPARAM);
}