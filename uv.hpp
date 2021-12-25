#pragma once

#include "game.hpp"
#include "transform.hpp"
#include "buffer.hpp"

namespace Win32GameEngine {
	struct RectBound {
		using V = Vec2F;
		V min, max;
		RectBound() : min{ INFINITY, INFINITY }, max{ -INFINITY, -INFINITY } {}
		RectBound(V min, V max) : RectBound(){
			add(min);
			add(max);
		}
		RectBound(RectBound const &r) : RectBound(V(r.min), V(r.max)) {}
		V topleft() const { return min; }
		V topright() const { return { max[0], min[1] }; }
		V bottomright() const { return max; }
		V bottomleft() const { return { min[0], max[1] }; }
		void add(V point) {
			min[0] = std::min(point[0], min[0]);
			min[1] = std::min(point[1], min[1]);
			max[0] = std::max(point[0], max[0]);
			max[1] = std::max(point[1], max[1]);
		}
		RectBound transform(function<V(V)> f) {
			RectBound res;
			res.add(f(topleft()));
			res.add(f(topright()));
			res.add(f(bottomleft()));
			res.add(f(bottomright()));
			return res;
		}
		RectBound clip(RectBound r) {
			RectBound res;
			res.min[0] = std::max(r.min[0], min[0]);
			res.min[1] = std::max(r.min[1], min[1]);
			res.max[0] = std::min(r.max[0], max[0]);
			res.max[1] = std::min(r.max[1], max[1]);
			return res;
		}
	};
	
	class UV : public Component {
	public:
		RectBound bound;
		Vec2F size, anchor;
		UV(Entity *entity, Vec2F size, Vec2F anchor) :
			Component(entity), size(size), anchor(anchor),
			bound(RectBound(anchor * -1, size - anchor)) {
		}
		virtual bool hit(Vec2F uv) const {
			float x = uv[0], y = uv[1];
			return (x < bound.max[0]) && (x >= bound.min[0]) && (y < bound.max[1]) && (y >= bound.min[1]);
		}
		virtual Color sample(Vec2F uv) const = 0;
	};

	class SolidBlock : public UV {
	public:
		Color color;
		SolidBlock(Entity *entity, Color color, Vec2F size, Vec2F anchor) :
			UV(entity, size, anchor), color(color) {
		}
		SolidBlock(Entity *entity, Color color, Vec2F size) : SolidBlock(entity, color, size, size * .5f) {}
		inline virtual Color sample(Vec2F uv) const override { return color; }
	};

	class Sprite : public UV {
	public:
		Bitmap bitmap;
		Sprite(Entity *entity, Bitmap const &bitmap, Vec2F anchor) :
			UV(entity, bitmap.dimension, anchor), bitmap(bitmap) {
		}
		Sprite(Entity *entity, Bitmap const &bitmap) : Sprite(entity, bitmap, bitmap.dimension * .5f) {}
		inline virtual Color sample(Vec2F uv) const override {
			Color *color = bitmap.at(uv + anchor);
			if(!color)
				return Color();
			return *color;
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