#pragma once

#include "game.hpp"
#include "render.hpp"

namespace Win32GameEngine {
	class UI : public Renderer {
	protected:
		virtual bool validate(Entity const *entity) override {
			if(!entity->isactive())
				return false;
			if(!dynamic_cast<ScreenEntity const *>(entity))
				return false;
			Texture *const texture = entity->getcomponent<Texture>();
			if(!texture || !texture->isactive())
				return false;
			return true;
		}
		virtual bool compare(Entity const *a, Entity const *b) override {
			return ((ScreenEntity *)a)->transform->z.value < ((ScreenEntity *)b)->transform->z.value;
		}
		virtual void sample() override {
			for(Entity *const entity : queue) {
				ScreenEntity *se = (ScreenEntity *)entity;
				Texture *const texture = entity->getcomponent<Texture>();
				Bound bound = texture->bound.transform([&](Vec2F v) {
					Vec3F a = v;
					a[2] = 1;
					return se->transform->world(a);
				});
				texture->put(target.getdc(), bound);
			}
		}
	public:
		UI(Entity *entity) : Renderer(entity) {}
	};

	class UIBase : public ScreenEntity {
	public:
		UI &ui;
		UIBase(Scene *scene) : ScreenEntity(scene),
			ui(*makecomponent<UI>()) {
		}
	};
}