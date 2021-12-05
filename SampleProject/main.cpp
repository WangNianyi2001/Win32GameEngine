#include "win32ge.hpp"

using namespace Win32GameEngine;

class MyGame : public Game {
	static inline Vec2U const size = { 400, 400 };
public:
	MyGame() : Game(new Window({
		.background_brush = CreateSolidBrush(RGB(255, 0, 0)),
		.size = size,
		.style = Window::Style::CENTERED
	})) {
		window->events.add(WM_ACTIVATE, [&](SystemEvent event) {
			InvalidateRect(event.data.hWnd, nullptr, false);
			return 0;
		});
		window->events.add(WM_PAINT, [&](SystemEvent event) {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(event.data.hWnd, &ps);
			Camera camera(size);
			camera.resample();
			camera.painton(hdc);
			EndPaint(event.data.hWnd, &ps);
			return 0;
		});
		window->events.add(WM_DESTROY, [&](SystemEvent) {
			quit();
			return 0;
		});
	}
};

Game *Win32GameEngine::game = new MyGame();