#pragma once

#include "game.hpp"

namespace Win32GameEngine {
	template<unsigned D>
	class Transform : public Component {
		using Matrix = SquareMatrix<D, float>;
	protected:
		Transform *parent;
		set<Transform *> children;
		virtual void updatelocal() = 0;
		virtual void updateworld() {
			world = parent ? parent->world.compose(local) : local;
		}
		virtual void update() = 0;
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
		Matrix local, world;
		Transform(Entity *entity) : Component(entity) {}
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

	struct WorldTransform : Transform<4> {
	protected:
		WorldTransform *parent;
		set<WorldTransform *> children;
		inline void updateworld() {
			world = parent ? parent->world.compose(local) : local;
		}
	public:
		Attribute<Vec3F> position;
		Attribute<float> rotation;
		Attribute<Vec3F> scale;
		WorldTransform(Entity *entity) : Transform(entity),
			position(this, Vec3F{ 0, 0, 0 }),
			rotation(this, .0f),
			scale(this, { 1, 1, 1 }),
			parent(nullptr), children() {
			update();
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
		virtual void update() override {
			updatelocal();
			updateworld();
			for(WorldTransform *child : children)
				child->updateworld();
		}
	};

	using WorldEntity = TransformEntity<WorldTransform>;

	class ScreenTransform : public Transform<3> {
		ScreenTransform *parent;
		set<ScreenTransform *> children;
		inline void updateworld() {
			world = parent ? parent->world.compose(local) : local;
		}
	public:
		Attribute<Vec2F> position;
		Attribute<float> z;
		Attribute<Vec2F> scale;
		ScreenTransform(Entity *entity) : Transform(entity),
			position(this, { 0, 0 }),
			z(this, 0),
			scale(this, { 1, 1 }) {
			update();
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
		virtual void update() override {
			updatelocal();
			updateworld();
			for(ScreenTransform *child : children)
				child->updateworld();
		}
	};

	using ScreenEntity = TransformEntity<ScreenTransform>;
}