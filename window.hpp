#pragma once

#include "basics.hpp"
#include <windows.h>
#include "event.hpp"

namespace Win32GameEngine {
	using namespace std;

	struct SystemEventData {
		HWND handle;
		WPARAM wParam;
		LPARAM lParam;
	};
	struct SystemEvent : Event<UINT, SystemEventData> {
		void defaultBehavior() {
			DefWindowProc(data.handle, type, data.wParam, data.lParam);
		}
	};
	using SystemHandler = Handler<SystemEvent>;

	class Window {
	public:
		struct Distributor : EventDistributor<SystemEvent> {
			virtual void miss(SystemEvent event) override {
				event.defaultBehavior();
			}
		};
		Distributor events;
		enum class Style {
			ASIS, CENTERED, FULLSCREEN
		};
		struct InitArg {
			ConstString class_name = L"Window";
			HINSTANCE instance = nullptr;
			ConstString title = nullptr;
			UINT class_style = CS_HREDRAW | CS_VREDRAW;
			HICON icon = nullptr, icon_small = nullptr;
			HCURSOR cursor = nullptr;
			HBRUSH background_brush = (HBRUSH)GetStockObject(BLACK_BRUSH);
			ConstString menu_name = nullptr;
			Vec2I position{ CW_USEDEFAULT, CW_USEDEFAULT };
			Vec2U size{ 640, 480 };
			Style style = Style::ASIS;
		};
		static inline Vec2I screen{ GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
	private:
		using HWndMap = map<HWND, Window *>;
		inline static HWndMap hwnd_map = HWndMap();
		HWND handle;
		InitArg args;
		static LRESULT CALLBACK event_processor(HWND handle, UINT type, WPARAM w, LPARAM l) {
			auto it = Window::hwnd_map.find(handle);
			if(it == Window::hwnd_map.end())
				return DefWindowProc(handle, type, w, l);
			it->second->events({ type, EventPropagation::NONE, SystemEventData{ handle, w, l }});
			return 0;
		}
	public:
		Window(InitArg const args) : args(args) {
			WNDCLASS window_class = {
				.style = args.class_style,
				.lpfnWndProc = &Window::event_processor,
				.cbClsExtra = 0,
				.cbWndExtra = 0,
				.hInstance = args.instance,
				.hIcon = args.icon,
				.hCursor = args.cursor,
				.hbrBackground = args.background_brush,
				.lpszMenuName = args.menu_name,
				.lpszClassName = args.class_name,
			};
			RegisterClass(&window_class);
			handle = CreateWindow(
				args.class_name, args.title, WS_POPUP,
				args.position.at(0), args.position.at(1),
				args.size.at(0), args.size.at(1),
				nullptr, nullptr, nullptr, nullptr
			);
			hwnd_map.insert(pair(handle, this));
		}
		void init() {
			ShowWindow(handle, SW_SHOW);
			switch(args.style) {
			case Style::CENTERED:
				center();
				break;
			case Style::FULLSCREEN:
				fullscreen();
				break;
			}
			UpdateWindow(handle);
		}
		void update() {
			MSG message;
			while(PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
		void fullscreen() {
			args.size = screen;
			SetWindowPos(handle, nullptr, 0, 0, screen[0], screen[1], SWP_SHOWWINDOW);
		}
		void center() {
			Vec2I pos = (screen - args.size) * .5f;
			MoveWindow(handle, pos[0], pos[1], args.size[0], args.size[1], FALSE);
		}
		void minimize() {
			ShowWindow(handle, SW_MINIMIZE);
		}
		void restore() {
			ShowWindow(handle, SW_RESTORE);
			BringWindowToTop(handle);
		}
		void invalidate(bool erase = false) {
			InvalidateRect(handle, nullptr, erase);
		}
	};

	auto defaultQuit = [](SystemEvent) {
		PostQuitMessage(0);
		return 0;
	};
}