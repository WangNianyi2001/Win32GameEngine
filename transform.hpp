#pragma once

#include "game.hpp"

namespace Win32GameEngine {
	template<unsigned D, typename Impl>
	class Transform : public Component {
	public:
		using Matrix = SquareMatrix<D, float>;
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
		Matrix local, world;
		Transform *parent;
		set<Transform *> children;
		Transform(Entity *entity) : Component(entity),
			parent(nullptr), children() {
			this->update();
		}
		virtual void updatelocal() = 0;
		virtual void updateworld() {
			world = parent ? parent->world.compose(local) : local;
		}
		void update() {
			((Impl *)this)->Impl::updatelocal();
			updateworld();
			for(Transform *child : children)
				child->updateworld();
		}
	};

	template<typename T>
	class TransformEntity : public Entity {
	public:
		T &transform;
		TransformEntity(Scene *scene) : Entity(scene),
			transform(*makecomponent<T>()) {
			scene->addentity(this);
		}
	};

	struct WorldTransform : public Transform<4, WorldTransform> {
	public:
		Attribute<Vec3F> position;
		Attribute<float> rotation;
		Attribute<Vec3F> scale;
		WorldTransform(Entity *entity) : Transform(entity),
			position(this, Vec3F{ 0, 0, 0 }),
			rotation(this, .0f),
			scale(this, { 1, 1, 1 }) {
		}
		void setparent(WorldTransform *t) {
			if(parent)
				parent->children.erase(this);
			if(parent = t)
				parent->children.insert(this);
			updateworld();
		}
		virtual void updatelocal() override {
			local.diag() = Vec4F{ 1, 1, 1, 1 };
			float rot = rotation();
			Vec3F scale = this->scale();
			float c = cos(rot), s = sin(rot);
			float x = scale[0], y = scale[1];
			local.row(0) = Vec2F{ x * c, -y * s };
			local.row(1) = Vec2F{ x * s, y * c };
			local.col(3) = position();
		}
	};

	using WorldEntity = TransformEntity<WorldTransform>;

	class ScreenTransform : public Transform<3, ScreenTransform> {
	public:
		Attribute<Vec2F> position;
		Attribute<float> z;
		Attribute<Vec2F> scale;
		ScreenTransform(Entity *entity) : Transform(entity),
			position(this, { 0, 0 }),
			z(this, 0),
			scale(this, { 1, 1 }) {
		}
		void setparent(ScreenTransform *t) {
			if(parent)
				parent->children.erase(this);
			if(parent = t)
				parent->children.insert(this);
			updateworld();
		}
		virtual void updatelocal() override {
			Vec3F diag = this->scale();
			diag[2] = 1;
			local.diag() = diag;
			local.col(2) = position();
		}
	};

	using ScreenEntity = TransformEntity<ScreenTransform>;
}