#pragma comment(lib, "winmm.lib")

#include "sound.h"

using namespace Win32GameEngine;

void Win32GameEngine::playSound(wchar_t const *url, int option) {
	PlaySound(url, NULL, option);
}