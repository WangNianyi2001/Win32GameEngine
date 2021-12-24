#pragma once

#include "game.hpp"
#include "transform.hpp"
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
			//float z = -camera_entity(top)[2] / camera_entity(bottom)[2];
			float z = -camera_entity.data[11];
			top = top * z + bottom;
			return camera_entity(top);
		}
		static Vec2F uv_screen(
			SquareMatrix<4, float> const &entity_camera, Vec2F uvp
		) {
			Vec4F augmented = uvp;
			augmented[3] = 1;
			Vec4F camerap = entity_camera(augmented);
			return camerap * (1 / camerap[2]);
		}
		Vec2I buffer_shift;
		float pixel_scale;
	public:
		Camera(Entity *entity, Vec2U dimension, float view_size) :
			Component(entity), Bitmap(dimension) {
			buffer_shift = Vec2F(dimension) * .5f;
			setviewsize(view_size);
			add(GameEventType::PAINT, [=](GameEvent) {
				Scene *scene = entity->scene;
				sort(scene->solid_entities.begin(), scene->solid_entities.end(), [](Entity *a, Entity *b) {
					float
						az = a->getcomponent<Transform>()->position.value[2],
						bz = b->getcomponent<Transform>()->position.value[2];
					return az > bz;
				});
				sample();
				HDC hdc = scene->game->gethdc();
				HDC com = CreateCompatibleDC(hdc);
				SelectObject(com, gethandle());
				BitBlt(hdc, 0, 0, dimension[0], dimension[1], com, 0, 0, SRCCOPY);
				DeleteObject(com);
			});
		}
		inline float setviewsize(float view_size) {
			return pixel_scale = view_size / dimension.module();
		}
		inline float setfov(float fov) { setviewsize(tan(fov)); }
		inline float setfovindegree(int fov) { setviewsize(tan(fov * (atan(1) / 90))); }
		inline Vec2I screen_buffer(Vec2F screenp) const {
			return screenp * (1 / pixel_scale) + buffer_shift;
		}
		inline Vec2F buffer_screen(Vec2I bufferp) const {
			return (bufferp - buffer_shift) * pixel_scale;
		}
		void sample() {
			clear();
			Transform &self_transform = *entity->getcomponent<Transform>();
			RectBound screen_clip{ buffer_screen({ 0, 0 }), buffer_screen(dimension) };
			for(Entity *const entity : entity->scene->solid_entities) {
				UV *const uv = entity->getcomponent<UV>();
				if(!uv)
					continue;
				if(uv->entity->getcomponent<Transform>()->position.value[2] <= 0)
					continue;
				SquareMatrix<4, float>
					camera_entity = entity->getcomponent<Transform>()->inverse.compose(self_transform),
					entity_camera = camera_entity.inverse();
				RectBound screenb = uv->bound
					.transform(bind_front(uv_screen, entity_camera))
					.clip(screen_clip);
				float const
					ymin = screenb.min[1],
					ymax = screenb.max[1],
					xmin = screenb.min[0],
					xmax = screenb.max[0];
				for(float y = ymin; y < ymax; y += pixel_scale) {
					for(float x = xmin; x < xmax; x += pixel_scale) {
						Vec2F screenp{ x, y };
						Color *target = at(screen_buffer(screenp));
						if(!target)
							continue;
						Vec2F uvp = screen_uv(camera_entity, screenp);
						if(uv->hit(uvp))
							*target = *target + uv->sample(uvp);
					}
				}
			}
		}
	};
}