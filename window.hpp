#pragma once

#include <windows.h>
#include "event.hpp"
#include "gameobject.hpp"

namespace Win32GameEngine {
	using namespace std;

	using RawHandler = Handler<LRESULT, HWND, WPARAM, LPARAM>;

	static RawHandler *defaultDestroy = new RawHandler{ [](HWND, WPARAM, LPARAM) {
		PostQuitMessage(0);
		return (LRESULT)0;
	} };

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

	struct Painter : EventReceiverWrapper<PaintMedium, LRESULT, HWND, WPARAM, LPARAM> {
		Painter(function<void(HDC)> f) : EventReceiverWrapper<
			PaintMedium, LRESULT, HWND, WPARAM, LPARAM
		>(PaintMedium(new Handler<void, HDC>{ f })) {}
	};

	using String = LPTSTR;
	using ConstString = LPCTSTR;

	class Window : public GameObject {
		static LRESULT CALLBACK event_processor(HWND, RawEvent, WPARAM, LPARAM);
		using HWndMap = map<HWND, Window *>;
		inline static HWndMap hwnd_map = HWndMap();
	private:
		HWND hWnd;
	public:
		struct Distributor : public EventDistributor<
			RawEvent,
			EventReceiver<LRESULT, HWND, WPARAM, LPARAM>,
			LRESULT, HWND, RawEvent, WPARAM, LPARAM
		> {
			virtual LRESULT operator()(HWND hWnd, RawEvent type, WPARAM w, LPARAM l) override {
				auto it = receivers.find(type);
				if(it == receivers.end())
					return DefWindowProc(hWnd, type, w, l);
				auto typed = it->second;
				LRESULT res = 0;
				for(auto receiver : typed)
					res |= receiver->operator()(hWnd, w, l);
				return res;
			}
		};
		Distributor event_distributor;
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
		Window(InitArg const init_args);
		virtual void init() override;
		virtual void update() override;
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