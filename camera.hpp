#pragma once

#include "buffer.hpp"
#include "game.hpp"

namespace Win32GameEngine {
	class Camera : public Component {
	public:
		using Index = Bitmap::_Index;
		Camera(Entity *parent) : Component(parent) {}
		virtual void onpaint() override {
			//
		}
	};
}