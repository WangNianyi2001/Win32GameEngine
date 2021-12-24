#pragma once

#include "game.hpp"

namespace Win32GameEngine {
	struct Transform : Component, AffineMatrix<3, float> {
		template<typename T>
		struct Attribute {
			Transform *const transform;
			T value;
			Attribute(Transform *t, T const &v) : transform(t) {
				operator=(v);
			}
			inline T operator()() { return value; }
			Attribute<T> &operator=(T const &v) {
				value = v;
				transform->update();
				return *this;
			}
		};
		Attribute<Vec3F> position;
		Attribute<float> rotation;
		Attribute<Vec3F> scale;
		Transform(Entity *entity) : Component(entity), position(this, { 0, 0, 0 }), rotation(this, .0f), scale(this, { 1, 1, 1 }) {
			row(2)[2] = 1;
			entity->scene->solid_entities.push_back(entity);
		}
		~Transform() {
			auto &s = entity->scene->solid_entities;
			auto it = find(s.begin(), s.end(), entity);
			if(it != s.end())
				s.erase(it);
		}
		AffineMatrix<3, float> inverse;
	protected:
		friend Attribute<Vec3F>;
		friend Attribute<float>;
		void update() {
			float rot = rotation();
			Vec3F sca = scale();
			float c = cos(rot), s = sin(rot);
			float x = sca[0], y = sca[1];
			row(0) = Vec2F{ x * c, -y * s };
			row(1) = Vec2F{ x * s, y * c };
			col(3) = position();
			inverse = AffineMatrix<3, float>::inverse();
		}
	};
}