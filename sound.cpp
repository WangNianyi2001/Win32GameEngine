#pragma comment(lib, "winmm.lib")

#include "sound.h"
#include <windows.h>
#include <mmsystem.h>

using namespace Win32GameEngine;

void Win32GameEngine::playSound(wchar_t const *url) {
	PlaySound(url, NULL, SND_ASYNC);
}
