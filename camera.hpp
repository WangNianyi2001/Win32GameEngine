#pragma once

#include "game.hpp"
#include "render.hpp"

namespace Win32GameEngine {
	class Camera : public Renderer {
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
		virtual bool compare(Entity const *a, Entity const *b) override {
			float
				az = ((WorldEntity const *)a)->transform.position.value[2],
				bz = ((WorldEntity const *)b)->transform.position.value[2];
			return az > bz;
		}
		virtual bool validate(Entity const *entity) override {
			if(!entity->isactive())
				return false;
			if(!dynamic_cast<WorldEntity const *>(entity))
				return false;
			Texture *const texture = entity->getcomponent<Texture>();
			if(!texture || !texture->isactive())
				return false;
			return true;
		}
		inline Vec2I screen_buffer(Vec2F screenp) const {
			return screenp * (1 / pixel_scale) + buffer_shift;
		}
		inline Vec2F buffer_screen(Vec2I bufferp) const {
			return (bufferp - buffer_shift) * pixel_scale;
		}
		virtual void sample() override {
			WorldTransform &self_transform = *entity->getcomponent<WorldTransform>();
			Bound screen_clip{
				buffer_screen({ 0, 0 }),
				buffer_screen(target.dimension)
			};
			for(Entity *const entity : queue) {
				Texture *const texture = entity->getcomponent<Texture>();
				SquareMatrix<4, float>camera_entity =
					((WorldEntity const *)entity)
					->transform.world.inverse()
					.compose(self_transform.world);
				Bound screenb = texture->bound
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
	public:
		Camera(Entity *entity, float view_size) : Renderer(entity),
			buffer_shift(Vec2F(target.dimension) * .5f) {
			setviewsize(view_size);
		}
		inline float setviewsize(float view_size) {
			return pixel_scale = view_size / target.dimension.module();
		}
		inline float setfov(float fov) { setviewsize(tan(fov)); }
		inline float setfovindegree(int fov) {
			setviewsize((float)tan(fov * (atan(1) / 90)));
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