#pragma once

#include "game.hpp"
#include "texture.hpp"

namespace Win32GameEngine {
	class Renderer : public Component {
	protected:
		Bitmap &target;
		Renderer(Entity *entity) : Component(entity),
			target(entity->scene->game->buffer) {
		}
		virtual void sample() = 0;
		inline void clear() {
			memset(target.data.get(), 0, target.size * sizeof(Color));
		}
	};
}

#include "camera.hpp"
#include "ui.hpp"