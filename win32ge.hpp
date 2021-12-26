#pragma once

#define WIN32_LEAN_AND_MEAN

#include "game.hpp"
#include "vanilla.hpp"

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
		for(game->start(); game->isactive(); game->update());
	} catch(ConstString msg) {
		MessageBox(game->window->handle, msg, L"Error", MB_OK);
	}
	DestroyWindow(game->window->handle);
	return 0;
}