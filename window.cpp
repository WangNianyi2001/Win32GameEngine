#include "window.hpp"

using namespace Win32GameEngine;
using namespace std;

LRESULT CALLBACK Window::event_processor(HWND hWnd, RawEvent type, WPARAM w, LPARAM l) {
	auto it = Window::hwnd_map.find(hWnd);
	if(it == Window::hwnd_map.end())
		return DefWindowProc(hWnd, type, w, l);
	return it->second->event_distributor(hWnd, type, w, l);
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
	hwnd_map.insert(pair(hWnd, this));
	if(args.position == Position::CENTERED)
		this->center();
}

void Window::init() {
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
}

void Window::update() {
	MSG message;
	while(PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}
