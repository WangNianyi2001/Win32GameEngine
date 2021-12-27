#pragma once

#include "game.hpp"
#include "render.hpp"

namespace Win32GameEngine {
	class Camera : public Renderer {
	protected:
		virtual Vec2F screen_texture(Texture const *texture, Vec2F screenp) const override {
			SquareMatrix<4, float> camera_entity = ((WorldEntity const *)texture->entity)
				->transform.world.inverse()
				.compose(entity->getcomponent<WorldTransform>()->world);
			Vec4F top = screenp;
			top[2] = 1;
			Vec4F bottom{ 0, 0, 0, 1 };
			//float z = -camera_entity(top)[2] / camera_entity(bottom)[2];
			// Technically incorrect, but works for camera-aligned cases.
			float z = -camera_entity.data[11];
			top = top * z + bottom;
			return camera_entity(top);
		}
		virtual Vec2F texture_screen(
			Texture const *texture, Vec2F texturep 
		) const override {
			SquareMatrix<4, float> entity_camera = ((WorldEntity const *)texture->entity)
				->transform.world.inverse()
				.compose(entity->getcomponent<WorldTransform>()->world);
			entity_camera = entity_camera.inverse();
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
		virtual inline Vec2I screen_buffer(Vec2F screenp) const override {
			return screenp * (1 / pixel_scale) + buffer_shift;
		}
		virtual inline Vec2F buffer_screen(Vec2I bufferp) const override {
			return (bufferp - buffer_shift) * pixel_scale;
		}
		virtual void sample() override {
			WorldTransform &self_transform = *entity->getcomponent<WorldTransform>();
			for(Entity *const entity : queue) {
				Texture *const texture = entity->getcomponent<Texture>();
				SquareMatrix<4, float>camera_entity =
					((WorldEntity const *)entity)
					->transform.world.inverse()
					.compose(self_transform.world);
				Bound screenb = texture->bound.transform([=](Vec2F texturep) {
					return texture_screen(texture, texturep);
				});
				float const
					ymin = screenb.min[1],
					ymax = screenb.max[1],
					xmin = screenb.min[0],
					xmax = screenb.max[0];
				for(float y = ymin; y < ymax; y += pixel_scale) {
					for(float x = xmin; x < xmax; x += pixel_scale) {
						Vec2F screenp{ x, y }, bufferp = screen_buffer(screenp);
						Color *pixel = buffer.at(bufferp);
						if(!pixel)
							continue;
						Vec2F texturep = screen_texture(texture, screenp);
						if(texture->hit(texturep)) {
							*pixel = *pixel + texture->sample(texturep);
							int a = 1;
						}
					}
				}
			}
		}
	public:
		Camera(Entity *entity, float view_size) : Renderer(entity),
			buffer_shift(Vec2F(target().dimension) * .5f) {
			setviewsize(view_size);
		}
		inline float setviewsize(float view_size) {
			return pixel_scale = view_size / buffer.dimension.module();
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