#pragma once

#include "win32gecore.hpp"
#include <map>
#include <set>
#include <functional>

namespace Win32GameEngine {
	using namespace std;

	using Event = UINT;

	template<typename Out, typename ...In>
	struct EventReceiver {
		virtual Out operator()(In ...args) = 0;
	};

	template<typename Out, typename ...In>
	struct Handler : EventReceiver<Out, In ...> {
		using Function = function<Out(In ...)>;
		Function const f;
		virtual Out operator()(In ...args) {
			return f(args...);
		}
		Handler(Function f) : f(f) {}
	};

	using PureHandler = Handler<LRESULT, HWND, WPARAM, LPARAM>;
	
	static PureHandler *defaultDestroy = new PureHandler{
		[](HWND, WPARAM, LPARAM) {
			PostQuitMessage(0);
			return (LRESULT)0;
		}
	};

	template<typename Next, typename Out, typename ...In>
	struct EventMedium : EventReceiver<Out, In ...> {
		Next const next;
		virtual Out operator()(In ...args) = 0;
		EventMedium(Next next) : next(next) {}
	};

	struct EventDistributor : EventReceiver<LRESULT, HWND, Event, WPARAM, LPARAM> {
		using Receiver = EventReceiver<LRESULT, HWND, WPARAM, LPARAM>;
		template<typename T>
		using Container = map<Event, set<T>>;
		Container<Receiver *> receivers;
		EventDistributor() = default;
		virtual LRESULT operator()(HWND hWnd, Event type, WPARAM w, LPARAM l);
		void add(Event type, Receiver *receiver);
	};

	class Window {
		static LRESULT CALLBACK event_processor(HWND, Event, WPARAM, LPARAM);
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
}