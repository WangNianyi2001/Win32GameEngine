#pragma once

#include <windows.h>
#include "event.hpp"
#include "gameobject.hpp"

namespace Win32GameEngine {
	using namespace std;

	using RawEventData = struct {
		HWND hWnd;
		WPARAM wParam;
		LPARAM lParam;
	};
	using RawEvent = Event<UINT, RawEventData>;
	using RawHandler = Handler<LRESULT, RawEvent>;
	static RawHandler *defaultDestroy = new RawHandler{ [](RawEvent) {
		PostQuitMessage(0);
		return (LRESULT)0;
	} };

	/*
	struct PaintMedium : EventMedium<
		EventReceiver<void, HDC> *,
		LRESULT, HWND, WPARAM, LPARAM
	> {
		using EventMedium<
			EventReceiver<void, HDC> *,
			LRESULT, HWND, WPARAM, LPARAM
		>::EventMedium;
		virtual LRESULT operator()(HWND hWnd, WPARAM, LPARAM) override {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			next->operator()(hdc);
			EndPaint(hWnd, &ps);
			return 0;
		}
	};

	struct Painter : EventExecutor<PaintMedium, LRESULT, HWND, WPARAM, LPARAM> {
		Painter(function<void(HDC)> f) : EventExecutor<
			PaintMedium, LRESULT, HWND, WPARAM, LPARAM
		>(PaintMedium(new Handler<void, HDC>{ f })) {}
	};
	*/

	using String = LPTSTR;
	using ConstString = LPCTSTR;

	class Window {
		static LRESULT CALLBACK event_processor(HWND hWnd, UINT type, WPARAM w, LPARAM l) {
			auto it = Window::hwnd_map.find(hWnd);
			if(it == Window::hwnd_map.end())
				return DefWindowProc(hWnd, type, w, l);
			return it->second->events(RawEvent{
				type,
				EventPropragation::DISABLED,
				RawEventData{ hWnd, w, l }
			});
		}
		using HWndMap = map<HWND, Window *>;
		inline static HWndMap hwnd_map = HWndMap();
	private:
		HWND hWnd;
	public:
		struct Distributor : public EventDistributor<RawEvent, LRESULT> {
			virtual LRESULT operator()(RawEvent event) override {
				auto it = receivers.find(event.type);
				if(it == receivers.end())
					return DefWindowProc(event.data.hWnd, event.type, event.data.wParam, event.data.lParam);
				LRESULT res = 0;
				for(auto receiver : it->second)
					res |= receiver->operator()(event);
				return res;
			}
		};
		Distributor events;
		enum class Position {
			ASIS, CENTERED
		};
		struct InitArg {
			ConstString class_name = L"Window";
			HINSTANCE instance = nullptr;
			ConstString title = nullptr;
			UINT class_style = CS_HREDRAW | CS_VREDRAW;
			HICON icon = nullptr, icon_small = nullptr;
			HCURSOR cursor = nullptr;
			HBRUSH background_brush = nullptr;
			ConstString menu_name = nullptr;
			int x = 0, y = 0;
			int width = 640, height = 480;
			DWORD style = WS_OVERLAPPEDWINDOW;
			Position position = Position::ASIS;
		};
		Window(InitArg const args) {
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
				args.class_name, args.title, args.style,
				args.x, args.y,
				args.width, args.height,
				nullptr, nullptr, nullptr, nullptr
			);
			hwnd_map.insert(pair(hWnd, this));
			if(args.position == Position::CENTERED)
				this->center();
		}
		void init() {
			ShowWindow(hWnd, SW_SHOW);
			UpdateWindow(hWnd);
		}
		void update() {
			MSG message;
			while(PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
		void center() {
			RECT rect_self;
			GetWindowRect(hWnd, &rect_self);
			int width = rect_self.right - rect_self.left;
			int height = rect_self.bottom - rect_self.top;
			int screen_width = GetSystemMetrics(SM_CXSCREEN);
			int screen_height = GetSystemMetrics(SM_CYSCREEN);
			int x = (screen_width - width) / 2;
			int y = (screen_height - height) / 2;
			MoveWindow(hWnd, x, y, width, height, FALSE);
		}
	};
}