#include "window.h"

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

Window::Window(InitArg const init_args) : init_args(init_args) {
	WNDCLASS window_class = {
		.style = init_args.class_style,
		.lpfnWndProc = &Window::event_processor,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = init_args.instance,
		.hIcon = init_args.icon,
		.hCursor = init_args.cursor,
		.hbrBackground = init_args.background_brush,
		.lpszMenuName = init_args.menu_name,
		.lpszClassName = init_args.class_name,
	};
	RegisterClass(&window_class);
	hWnd = CreateWindow(
		init_args.class_name, init_args.title, init_args.style,
		init_args.x, init_args.y,
		init_args.width, init_args.height,
		nullptr, nullptr, nullptr, nullptr
	);
}

int Window::activate() {
	if(!hWnd)
		return 0;
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