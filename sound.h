#pragma once

#include <windows.h>
#include <mmsystem.h>

namespace Win32GameEngine {
	void playSound(wchar_t const *url, int option = SND_ASYNC);
	void playSoundEffect(wchar_t const *url);
}