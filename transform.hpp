#pragma once

#include "game.hpp"

namespace Win32GameEngine {
	struct Transform : Component {
	protected:
		Transform *parent;
		set<Transform *> children;
		inline void updateworld() {
			world = parent ? parent->world.compose(local) : local;
		}
	public:
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
		SquareMatrix<4, float> local, world;
		Transform(Entity *entity) : Component(entity),
			position(this, { 0, 0, 0 }), rotation(this, .0f), scale(this, { 1, 1, 1 }),
			parent(nullptr), children()
		{
			update();
		}
		void setparent(Transform *t) {
			if(parent)
				parent->children.erase(this);
			if(parent = t)
				parent->children.insert(this);
			updateworld();
		}
		void update() {
			local.diag() = Vec4F{ 1, 1, 1, 1 };
			float rot = rotation();
			Vec3F sca = scale();
			float c = cos(rot), s = sin(rot);
			float x = sca[0], y = sca[1];
			local.row(0) = Vec2F{ x * c, -y * s };
			local.row(1) = Vec2F{ x * s, y * c };
			local.col(3) = position();
			updateworld();
			for(Transform *child : children)
				child->updateworld();
		}
	};

	class SolidEntity : public Entity {
	public:
		Transform &transform;
		SolidEntity(Scene *scene) : Entity(scene),
			transform(*(Transform *)makecomponent<Transform>()) {
			scene->addentity(this);
		}
	};
}