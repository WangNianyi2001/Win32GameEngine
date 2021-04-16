#include "window.h"

using namespace Win32GameEngine;

HWND Window::createWindow(InitArg const &args) {
	WNDCLASSEX classex = {
		.cbSize = sizeof(WNDCLASSEX),
		.style = args.class_style,
		.lpfnWndProc = args.event_processor,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = hInstance,
		.hIcon = args.icon,
		.hCursor = args.cursor,
		.hbrBackground = args.background_brush,
		.lpszMenuName = args.menu_name,
		.lpszClassName = args.class_name,
		.hIconSm = args.icon_small
	};
	RegisterClassExW(&classex);
	HWND hWnd = CreateWindow(
		args.class_name, args.title, args.style,
		args.x, args.y,
		args.width, args.height,
		nullptr, nullptr, this->hInstance, nullptr
	);
	return hWnd;
}

Window::Window(HINSTANCE hInstance, InitArg const args) :
	args(args), hInstance(hInstance)
{}

int Window::run() {
	hWnd = createWindow(args);
	if(!hWnd)
		return FALSE;
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	MSG *message = new MSG;
	while(GetMessage(message, nullptr, 0, 0)) {
		TranslateMessage(message);
		DispatchMessage(message);
	}
	return message->wParam;
}

Win32GameEngine::EventHandler::EventHandler(map<UINT, set<Handler>> handlers) :
	handlers(handlers)
{}

LRESULT CALLBACK EventHandler::operator()(
	HWND hWnd, UINT type, WPARAM wParam, LPARAM lParam
) {
	auto &handlers = EventHandler::handlers;
	if(!handlers.count(type))
		return DefWindowProc(hWnd, type, wParam, lParam);
	LRESULT res = 0;
	for(Handler const handler : handlers[type])
		res |= handler(hWnd, wParam, lParam);
	return res;
}

void EventHandler::addHandler(UINT type, Handler handler) {
	if(!handlers.count(type))
		handlers.insert(std::pair(type, std::set<Handler>()));
	handlers[type].insert(handler);
}

LRESULT Win32GameEngine::defaultDestroyHandler(HWND, WPARAM, LPARAM) {
	PostQuitMessage(0);
	return 0;
}