#pragma once

#include "texture.h"
#include <windows.h>
#include <vector>
#include <functional>

namespace Win32GameEngine {
	using std::vector;
	using std::function;

	struct Frame {
		ULONGLONG interval;
		Texture texture;
	};

	struct Animation : public Scalable {
		ULONGLONG last_frame;
		vector<Frame> frames;
		vector<Frame>::iterator active;
		bool loop = false, stop = true;
		function<void(Animation *)> onEnd;
		Animation(vector<Frame> frames, bool loop = false, function<void(Animation *)> onEnd = nullptr);
		Animation(Animation const &reference);
		void begin();
		void update();
		void paintOn(HDC &hdc, Int2 position, Float2 scale = { 1.0f, 1.0f }) const;
	};
}