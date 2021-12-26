#pragma once

#include "render.hpp"
#include "world.hpp"

namespace Win32GameEngine {
	class Camera : public Renderer {
	protected:
		static Vec2F screen_texture(
			SquareMatrix<4, float> const &camera_entity, Vec2F screenp
		) {
			Vec4F top = screenp;
			top[2] = 1;
			Vec4F bottom{ 0, 0, 0, 1 };
			//float z = -camera_entity(top)[2] / camera_entity(bottom)[2];
			// Technically incorrect, but works for camera-aligned cases.
			float z = -camera_entity.data[11];
			top = top * z + bottom;
			return camera_entity(top);
		}
		static Vec2F texture_screen(
			SquareMatrix<4, float> const &entity_camera, Vec2F texturep
		) {
			Vec4F augmented = texturep;
			augmented[3] = 1;
			Vec4F camerap = entity_camera(augmented);
			return camerap * (1 / camerap[2]);
		}
		Vec2I buffer_shift;
		float pixel_scale;
		bool compare(Entity const *a, Entity const *b) {
			float
				az = ((WorldEntity const *)a)->transform.position.value[2],
				bz = ((WorldEntity const *)b)->transform.position.value[2];
			return az > bz;
		}
		static bool validate(Entity const *entity) {
			if(!entity->isactive())
				return false;
			WorldEntity const *world = dynamic_cast<WorldEntity const *>(entity);
			if(!world)
				return false;
			Texture *const texture = entity->getcomponent<Texture>();
			if(!texture || !texture->isactive())
				return false;
			return true;
		}
		vector<Entity *> queue;
	public:
		Camera(Entity *entity, float view_size) : Renderer(entity),
			queue() {
			buffer_shift = Vec2F(target.dimension) * .5f;
			setviewsize(view_size);
			add(GameEventType::PAINT, [=](GameEvent) {
				Scene *scene = entity->scene;
				// Sort solid entities by Z-order.
				queue.clear();
				copy_if(
					scene->entities.begin(),
					scene->entities.end(),
					back_inserter(queue),
					validate
				);
				sort(
					queue.begin(), queue.end(),
					bind(&Camera::compare, this, placeholders::_1, placeholders::_2)
				);
				// Perform rendering action.
				clear();
				sample();
				// Transfer onto device context.
				HDC hdc = scene->game->gethdc();
				HDC com = CreateCompatibleDC(hdc);
				SelectObject(com, target.gethandle());
				BitBlt(hdc, 0, 0, target.dimension[0], target.dimension[1], com, 0, 0, SRCCOPY);
				DeleteObject(com);
			});
		}
		inline float setviewsize(float view_size) {
			return pixel_scale = view_size / target.dimension.module();
		}
		inline float setfov(float fov) { setviewsize(tan(fov)); }
		inline float setfovindegree(int fov) {
			setviewsize((float)tan(fov * (atan(1) / 90)));
		}
		inline Vec2I screen_buffer(Vec2F screenp) const {
			return screenp * (1 / pixel_scale) + buffer_shift;
		}
		inline Vec2F buffer_screen(Vec2I bufferp) const {
			return (bufferp - buffer_shift) * pixel_scale;
		}
		virtual void sample() override {
			Transform &self_transform = *entity->getcomponent<Transform>();
			RectBound screen_clip{
				buffer_screen({ 0, 0 }),
				buffer_screen(target.dimension)
			};
			for(Entity *const entity : queue) {
				Texture *const texture = entity->getcomponent<Texture>();
				SquareMatrix<4, float>camera_entity =
					((WorldEntity const *)entity)
					->transform.world.inverse()
					.compose(self_transform.world);
				RectBound screenb = texture->bound
					.transform(
						bind_front(texture_screen, camera_entity.inverse())
					).clip(screen_clip);
				float const
					ymin = screenb.min[1],
					ymax = screenb.max[1],
					xmin = screenb.min[0],
					xmax = screenb.max[0];
				for(float y = ymin; y < ymax; y += pixel_scale) {
					for(float x = xmin; x < xmax; x += pixel_scale) {
						Vec2F screenp{ x, y };
						Color *pixel = target.at(screen_buffer(screenp));
						if(!pixel)
							continue;
						Vec2F texturep = screen_texture(camera_entity, screenp);
						if(texture->hit(texturep))
							*pixel = *pixel + texture->sample(texturep);
					}
				}
			}
		}
	};

	class CameraEntity : public WorldEntity {
	public:
		Camera &camera;
		CameraEntity(Scene *scene, float view_size) : WorldEntity(scene),
			camera(*makecomponent<Camera>(view_size))
		{}
	};
}