#pragma once

#include "game.hpp"

namespace Win32GameEngine {
	class Camera : public Component {
	public:
		Camera(Entity *parent) : Component(parent) {}
	};
}