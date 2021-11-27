#pragma once

#pragma warning(disable:4302)
#define WIN32_LEAN_AND_MEAN

#ifdef UNICODE
#define WIN32ENTRY_NAME wWinMain
#elif
#define WIN32ENTRY_NAME winMain
#endif

#define WIN32ENTRY int WINAPI WIN32ENTRY_NAME(\
	_In_ HINSTANCE hInstance,\
	_In_opt_ HINSTANCE hPrevInstance,\
	_In_ PWSTR pCmdLine,\
	_In_ int nCmdShow\
)

#include "window.hpp"