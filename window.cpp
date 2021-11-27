#include "window.hpp"

using namespace Win32GameEngine;
using namespace std;

EventDistributor::EventDistributor(HandlerMap handler_map) : handler_map(handler_map) {}

LRESULT CALLBACK EventDistributor::operator()(
	HWND hWnd, UINT type, WPARAM wParam, LPARAM lParam
) {
	if(!handler_map.count(type))
		return DefWindowProc(hWnd, type, wParam, lParam);
	LRESULT res = 0;
	for(Handler const handler : handler_map[type])
		res |= handler(hWnd, wParam, lParam);
	return res;
}

void EventDistributor::add(UINT type, Handler handler) {
	handler_map[type].insert(handler);
}

Window::HWndMap Window::hwnd_map = HWndMap();

LRESULT Window::event_processor(HWND hWnd, UINT type, WPARAM w, LPARAM l) {
	if(Window::hwnd_map.find(hWnd) == Window::hwnd_map.end())
		return DefWindowProc(hWnd, type, w, l);
	return Window::hwnd_map[hWnd]->event_distributor(hWnd, type, w, l);
}

Window::Window(InitArg const args) {
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
}

bool Window::ready() {
	return hWnd;
}

WPARAM Window::activate() {
	hwnd_map.insert(pair(hWnd, this));
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	MSG message;
	while(GetMessage(&message, nullptr, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	return message.wParam;
}

LRESULT Win32GameEngine::defaultDestroyHandler(HWND, WPARAM, LPARAM) {
	PostQuitMessage(0);
	return 0;
}