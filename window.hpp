#pragma once

#include "utils.hpp"

namespace Win32GameEngine {
	struct SystemEventData {
		HWND handle;
		WPARAM w;
		LPARAM l;
	};
	struct SystemEvent : Event<UINT> {
		SystemEventData data;
		inline LRESULT def() const {
			return DefWindowProc(data.handle, type, data.w, data.l);
		}
	};
	using SystemHandler = Handler<SystemEvent>;

	class Window {
	public:
		struct Distributor : EventDistributor<SystemEvent, LRESULT> {
			virtual inline LRESULT miss(SystemEvent const &event) override {
				return event.def();
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
			bool dpi_aware = true;
		};
		InitArg args;
		static inline Vec2I screen{
			GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN)
		};
	private:
		using HWndMap = map<HWND, Window *>;
		inline static HWndMap handles = HWndMap();
		static LRESULT CALLBACK event_processor(HWND handle, UINT type, WPARAM w, LPARAM l) {
			SystemEvent event{ type, Propagation::NONE, SystemEventData{ handle, w, l } };
			auto it = Window::handles.find(handle);
			if(it == Window::handles.end())
				return event.def();
			return it->second->events(event);
		}
	public:
		HWND handle;
		Bitmap buffer;
		Window(InitArg const args) : args(args), handle(NULL), buffer(args.size) {
			if(!args.dpi_aware)
				SetProcessDPIAware();
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
				nullptr, nullptr, nullptr, args.instance
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
	};
	
	auto _ = []() {};	// Ensure source compiling sequence
}