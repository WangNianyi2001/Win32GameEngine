#pragma once

#include "game.hpp"
#include "buffer.hpp"

namespace Win32GameEngine {
	template<typename T>
	struct RectBound {
		using V = Vector<2, T>;
		V min, max;
		RectBound() : min{ 0, 0 }, max{ 0, 0 } {}
		RectBound(V min, V max) : RectBound() {
			add(min);
			add(max);
		}
		template<derived_from_template<RectBound> R>
		RectBound(R const &r) : RectBound(V(r.min), V(r.max)) {}
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
		RectBound<T> transform(function<V(V)> f) {
			RectBound<T> res(f(min), f(max));
			res.add(f(topright()));
			res.add(f(bottomleft()));
			return res;
		}
	};
	
	class UV : public Component {
	public:
		UV(Entity *parent) : Component(parent) {
			//
		}
		RectBound<float> getbound() const {
			return RectBound<float>({ -1.f, -1.f }, { 1.f, 1.f });
		}
		Color sample(Vec2F uv) const {
			return Color{ 255, 0, 0, 255 };
		}
	};
}