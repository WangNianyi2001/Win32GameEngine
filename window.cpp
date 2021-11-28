#include "window.hpp"

using namespace Win32GameEngine;
using namespace std;

LRESULT EventDistributor::operator()(
	HWND hWnd, Event type, WPARAM w, LPARAM l
) {
	auto it = receivers.find(type);
	if(it == receivers.end())
		return DefWindowProc(hWnd, type, w, l);
	auto typed = it->second;
	LRESULT res = 0;
	for(Receiver *receiver : typed)
		res |= receiver->operator()(hWnd, w, l);
	return res;
}

void EventDistributor::add(Event type, Receiver *receiver) {
	receivers[type].insert(receiver);
}

Window::HWndMap Window::hwnd_map = HWndMap();

LRESULT Window::event_processor(HWND hWnd, Event type, WPARAM w, LPARAM l) {
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
