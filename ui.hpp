#pragma once

#include "game.hpp"

namespace Win32GameEngine {
	class UIBase : public Component {
	protected:
		Bitmap &target;
	public:
		UIBase(Entity *entity) : Component(entity),
			target(entity->scene->game->buffer)
		{
			//
		}
	};
}