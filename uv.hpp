#pragma once

#include "game.hpp"
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
	};
	
	class UV : public Component {
	public:
		UV(Entity *parent) : Component(parent) {}
		RectBound getbound() const {
			auto s = entity->transform.scale.value;
			return RectBound({ -s[0], -s[1] }, { s[0], s[1] });
		}
		virtual bool inbound(Vec2F uv) {
			RectBound bound = RectBound({ -1, -1 }, { 1, 1 });
			float x = uv[0], y = uv[1];
			return x < bound.max[0] && x >= bound.min[0] && y < bound.max[1] && y >= bound.min[1];
		}
		virtual Color sample(Vec2F uv) const = 0;
	};

	class PureBlock : public UV {
	public:
		Color color;
		PureBlock(Entity *parent) : UV(parent) {}
		PureBlock(Entity *parent, Color color) : UV(parent), color(color) {}
		inline virtual Color sample(Vec2F uv) const override {
			return color;
		}
	};
}