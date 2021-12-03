#include "win32ge.hpp"

using namespace Win32GameEngine;

class MyGame : public Game {
public:
	MyGame() : Game(new Window({
		.size = {100, 100},
		.style = Window::Style::CENTERED
	})) {
		window->events.add(WM_DESTROY, [&](SystemEvent) {
			quit();
			return 0;
		});
	}
};

Game *Win32GameEngine::game = new MyGame();