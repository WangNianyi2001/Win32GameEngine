#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "game.hpp"

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
	game->init();
	for(; game->isactive(); )
		game->update();
	return 0;
}