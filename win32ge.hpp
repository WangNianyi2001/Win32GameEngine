#pragma once

#define WIN32_LEAN_AND_MEAN

#include "game.hpp"
#include "camera.hpp"

namespace Win32GameEngine {
	extern Game *game;
}

#ifdef UNICODE
#define WIN32ENTRY wWinMain
#elif
#define WIN32ENTRY winMain
#endif

int WINAPI WIN32ENTRY(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PWSTR pCmdLine,
	_In_ int nCmdShow
) {
	using namespace Win32GameEngine;
	if(!game)
		return 0;
	try {
		game->activate();
		for(; game->isactive(); ) {
			game->resolve();
			game->operator()({ GameEventType::UPDATE });
		}
	} catch(ConstString msg) {
		MessageBox(game->window->handle, msg, L"Error", MB_OK);
	}
	return 0;
}