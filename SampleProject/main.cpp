#include "win32ge.hpp"

using namespace Win32GameEngine;

class MyGame : public Game {
public:
	ConstString name = L"My Game";
	MyGame() : Game(new Window({
		.title = name,
		.position = Window::Position::CENTERED
	})) {
		window->events.add(WM_DESTROY, [&](SystemEvent) {
			quit();
			return 0;
		});
	}
};

Game *Win32GameEngine::game = new MyGame();