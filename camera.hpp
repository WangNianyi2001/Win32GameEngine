#pragma once

#include "game.hpp"
#include "uv.hpp"

namespace Win32GameEngine {
	class Camera : public Component, Bitmap {
	protected:
		static Vec2F screen_uv(
			SquareMatrix<4, float> const &camera_entity, Vec2F screenp
		) {
			Vec4F top = screenp;
			top[2] = 1;
			Vec4F bottom{ 0, 0, 0, 1 };
			float z = camera_entity(top)[2] / camera_entity(bottom)[2];
			top = top * z + bottom;
			return camera_entity(top);
		}
		static Vec2F uv_screen(
			SquareMatrix<4, float> const &entity_camera, Vec2F uvp
		) {
			Vec4F camera = entity_camera(uvp);
			return camera * (1 / camera[2]);
		}
		Vec2I buffer_shift;
	public:
		Camera(Entity *entity, Vec2U dimension) :
			Component(entity), Bitmap(dimension) {
			buffer_shift = Vec2F(dimension) * .5f;
			add(GameEventType::PAINT, [=](GameEvent) {
				sample();
				HDC hdc = entity->scene->game->gethdc();
				HDC com = CreateCompatibleDC(hdc);
				SelectObject(com, gethandle());
				BitBlt(hdc, 0, 0, dimension[0], dimension[1], com, 0, 0, SRCCOPY);
			});
		}
		inline Vec2I screen_buffer(Vec2F screenp) const {
			return buffer_shift + screenp;
		}
		void sample() {
			clear();
			Entity::Transform &self_transformation = entity->transform;
			for(Entity *const entity : entity->scene->entities) {
				UV *const uv = entity->getcomponent<UV>();
				if(!uv)
					continue;
				SquareMatrix<4, float>
					camera_entity =
						entity->transform.inverse.compose(self_transformation),
					entity_camera = camera_entity.inverse();
				RectBound<float> screenb(uv->getbound().transform(
					bind_front(Camera::screen_uv, camera_entity)
				));
				float const
					ymin = screenb.min[1],
					xmin = screenb.min[0],
					ymax = screenb.max[1],
					xmax = screenb.max[0];
				for(float y = ymin; y < ymax; ++y) {
					for(float x = xmin; x < xmax; ++x) {
						Vec2F screenp{ y, x };
						Color *target = at(screen_buffer(screenp));
						if(!target)
							continue;
						*target = *target + uv->sample(
							screen_uv(camera_entity, { y, x })
						);
					}
				}
			}
		}
	};
}