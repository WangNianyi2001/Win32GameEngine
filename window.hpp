#pragma once

// Agent for a Win32 window. Used by class Game.

#include "basics.hpp"
#include <windows.h>
#include "event.hpp"

namespace Win32GameEngine {
	// Derive a struct from templated struct Event to store
	// events passed from Win32 API.
	struct SystemEventData {
		HWND handle;
		WPARAM wParam;
		LPARAM lParam;
	};
	struct SystemEvent : Event<UINT> {
		SystemEventData data;
		void defaultBehavior() {
			DefWindowProc(data.handle, type, data.wParam, data.lParam);
		}
	};
	using SystemHandler = Handler<SystemEvent>;

	// THE agent class. Provides initialization & simple operations on windows.
	// Distributes system events to custom handlers.
	class Window {
	public:
		struct Distributor : EventDistributor<SystemEvent> {
			virtual void miss(SystemEvent event) override {
				event.defaultBehavior();
			}
		};
		Distributor events;
		// Window styles, can be specified when constructing.
		// ASIS - set position to explicitly written value,
		// CENTERED - center window by the screen position,
		// FULLSCREEN - fill the entire screen with the window.
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
		InitArg args;	// Stores a copy of the initializing arguments for later reference
		static inline Vec2I screen{	// Screen size.
			GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN)
		};
	private:
		using HWndMap = map<HWND, Window *>;
		inline static HWndMap handles = HWndMap();	// Reference for callback routing.
		static LRESULT CALLBACK event_processor(HWND handle, UINT type, WPARAM w, LPARAM l) {
			// Look up for corresponding window by handle.
			// Had to implement this way due to the early age C-style API design.
			auto it = Window::handles.find(handle);
			if(it == Window::handles.end())
				return DefWindowProc(handle, type, w, l);	// If not found, process by default.
			it->second->events({ type, SystemEventData{ handle, w, l }});
			return 0;
		}
	public:
		HWND handle;
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
			handles.insert(pair(handle, this));
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

	// Ensure source compiling sequence
	auto _ = []() {};
}