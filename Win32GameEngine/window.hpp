#pragma once

#include "basics.h"
#include <windows.h>
#include "event.hpp"

namespace Win32GameEngine {
	using namespace std;

	struct SystemEventData {
		HWND hWnd;
		WPARAM wParam;
		LPARAM lParam;
	};
	struct SystemEvent : Event<UINT, SystemEventData> {
		void defaultBehavior() {
			DefWindowProc(data.hWnd, type, data.wParam, data.lParam);
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
			HBRUSH background_brush = (HBRUSH)GetStockObject(WHITE_BRUSH);
			ConstString menu_name = nullptr;
			Vec2I position{ CW_USEDEFAULT, CW_USEDEFAULT };
			Vec2I size{ 640, 480 };
			Style style = Style::ASIS;
		};
		static inline Vec2I screen{ GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
	private:
		using HWndMap = map<HWND, Window *>;
		inline static HWndMap hwnd_map = HWndMap();
		HWND hWnd;
		InitArg args;
		static LRESULT CALLBACK event_processor(HWND hWnd, UINT type, WPARAM w, LPARAM l) {
			auto it = Window::hwnd_map.find(hWnd);
			if(it == Window::hwnd_map.end())
				return DefWindowProc(hWnd, type, w, l);
			it->second->events({ type, EventPropagation::NONE, SystemEventData{ hWnd, w, l }});
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
			hWnd = CreateWindow(
				args.class_name, args.title, WS_POPUP,
				args.position.at(0), args.position.at(1),
				args.size.at(0), args.size.at(1),
				nullptr, nullptr, nullptr, nullptr
			);
			hwnd_map.insert(pair(hWnd, this));
		}
		void init() {
			ShowWindow(hWnd, SW_SHOW);
			switch(args.style) {
			case Style::CENTERED:
				center();
				break;
			case Style::FULLSCREEN:
				fullscreen();
				break;
			}
			UpdateWindow(hWnd);
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
			SetWindowPos(hWnd, nullptr, 0, 0, screen[0], screen[1], SWP_SHOWWINDOW);
		}
		void center() {
			Vec2I pos = (screen - args.size) * .5f;
			MoveWindow(hWnd, pos[0], pos[1], args.size[0], args.size[1], FALSE);
		}
		void minimize() {
			ShowWindow(hWnd, SW_MINIMIZE);
		}
		void restore() {
			ShowWindow(hWnd, SW_RESTORE);
			BringWindowToTop(hWnd);
		}
	};

	auto defaultQuit = [](SystemEvent) {
		PostQuitMessage(0);
		return 0;
	};
}