#pragma once

#include "game.hpp"
#include "render.hpp"

namespace Win32GameEngine {
	class UI : public Renderer {
	protected:
		set<ScreenEntity *> elements;
		virtual Vec2F screen_texture(Texture const *texture, Vec2F screenp) override {
			ScreenTransform tt = ((ScreenEntity *)texture->entity)->transform;
			Vec3F aug = screenp;
			aug[2] = 1;
			return tt.world.inverse()(aug);
		}
		virtual bool validate(Entity const *entity) override {
			if(!entity->isactive())
				return false;
			if(elements.find((ScreenEntity *)entity) == elements.end())
				return false;
			Texture *const texture = entity->getcomponent<Texture>();
			if(!texture || !texture->isactive())
				return false;
			return true;
		}
		virtual bool compare(Entity const *a, Entity const *b) override {
			return ((ScreenEntity *)a)->transform.z.value < ((ScreenEntity *)b)->transform.z.value;
		}
		virtual void sample() override {
			for(Entity *const entity : queue) {
				ScreenEntity *se = (ScreenEntity *)entity;
				Texture *const texture = entity->getcomponent<Texture>();
				Bound bound = texture->bound.transform([&](Vec2F v) {
					Vec3F a = v;
					a[2] = 1;
					return se->transform.world(a);
				});
				texture->put(buffer, bound);
				GetBitmapBits(buffer.gethandle(), buffer.size * sizeof(Color), buffer.data.get());
			}
		}
	public:
		UI(Entity *entity) : Renderer(entity) {}
		ScreenEntity *makeelement() {
			ScreenEntity *el = new ScreenEntity(entity->scene);
			elements.insert(el);
			return el;
		}
	};

	class UIBase : public ScreenEntity {
	public:
		UI &ui;
		UIBase(Scene *scene) : ScreenEntity(scene),
			ui(*makecomponent<UI>()) {
		}
	};
}