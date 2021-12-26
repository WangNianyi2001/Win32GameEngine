#pragma once

#include "game.hpp"
#include "transform.hpp"

namespace Win32GameEngine {
	struct Bound {
		using V = Vec2F;
		V min, max;
		Bound() : min{ INFINITY, INFINITY }, max{ -INFINITY, -INFINITY } {}
		Bound(V min, V max) : Bound() {
			add(min);
			add(max);
		}
		Bound(Bound const &r) : Bound(V(r.min), V(r.max)) {}
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
		Bound transform(function<V(V)> f) const {
			Bound res;
			res.add(f(topleft()));
			res.add(f(topright()));
			res.add(f(bottomleft()));
			res.add(f(bottomright()));
			return res;
		}
		Bound transform(SquareMatrix<3, float> m) const {
			return transform([&](Vec2F v) { return m(v); });
		}
		Bound clip(Bound r) const {
			Bound res;
			res.min[0] = std::max(r.min[0], min[0]);
			res.min[1] = std::max(r.min[1], min[1]);
			res.max[0] = std::min(r.max[0], max[0]);
			res.max[1] = std::min(r.max[1], max[1]);
			return res;
		}
	};

	class Texture : public Component {
	public:
		Vec2F size, anchor;
		Bound bound;
		Texture(Entity *entity, Vec2F size, Vec2F anchor) :
			Component(entity), size(size), anchor(anchor),
			bound(Bound(anchor * -1, size - anchor)) {
		}
		void setanchor(Vec2F a) {
			anchor = a;
			bound = Bound(a * -1, size - a);
		}
		virtual bool hit(Vec2F uv) const {
			float x = uv[0], y = uv[1];
			return (x < bound.max[0]) && (x >= bound.min[0]) && (y < bound.max[1]) && (y >= bound.min[1]);
		}
		virtual Color sample(Vec2F uv) const = 0;
		virtual void put(HDC hdc, Bound bound) = 0;
	};

	class ColorBox : public Texture {
	public:
		Color color;
		ColorBox(Entity *entity, Color color, Vec2F size, Vec2F anchor) :
			Texture(entity, size, anchor), color(color) {
		}
		ColorBox(Entity *entity, Color color, Vec2F size) : ColorBox(entity, color, size, size * .5f) {}
		inline virtual Color sample(Vec2F uv) const override { return color; }
		virtual void put(HDC hdc, Bound bound) override {
			// TODO
		}
	};

	static inline BLENDFUNCTION blend_function = {
		AC_SRC_OVER, 0, 255, AC_SRC_ALPHA
	};

	class Sprite : public Texture {
	public:
		Bitmap bitmap;
		Sprite(Entity *entity, Bitmap const &bitmap, Vec2F anchor) :
			Texture(entity, bitmap.dimension, anchor), bitmap(bitmap) {
		}
		Sprite(Entity *entity, Bitmap const &bitmap) : Sprite(entity, bitmap, bitmap.dimension * .5f) {}
		inline virtual Color sample(Vec2F uv) const override {
			Color *color = bitmap.at(uv + anchor);
			if(!color)
				return Color();
			return *color;
		}
		virtual void put(HDC hdc, Bound bound) override {
			Vec2I pos = bound.topleft(), size = bound.bottomright() - pos;
			AlphaBlend(
				hdc,
				pos[0], pos[1], size[0], size[1],
				bitmap.getdc(),
				0, 0, bitmap.dimension[0], bitmap.dimension[1],
				blend_function
			);
		}
	};

	class Renderer : public Component {
	protected:
		Bitmap &target;
		vector<Entity *> queue;
		Renderer(Entity *entity) : Component(entity),
			target(entity->scene->game->buffer),
			queue(),
			clear_on_paint(false) {
			add(GameEventType::PAINT, [=](GameEvent) {
				Scene *scene = entity->scene;
				queue.clear();
				copy_if(
					scene->entities.begin(),
					scene->entities.end(),
					back_inserter(queue),
					[&](Entity const *e) { return validate(e); }
				);
				sort(
					queue.begin(), queue.end(),
					[&](Entity const *a, Entity const *b) { return compare(a, b); }
				);
				if(clear_on_paint)
					clear();
				sample();
				transfer();
			});
		}
		virtual bool validate(Entity const *entity) = 0;
		virtual bool compare(Entity const *a, Entity const *b) = 0;
		inline void clear() {
			memset(target.data.get(), 0, target.size * sizeof(Color));
		}
		virtual void sample() = 0;
		void transfer() {
			Bitmap &buffer = entity->scene->game->buffer;
			Vec2I bs = buffer.dimension, s = target.dimension;
			AlphaBlend(
				buffer.getdc(),
				0, 0, bs[0], bs[1],
				target.getdc(),
				0, 0, s[0], s[1],
				blend_function
			);
		}
	public:
		bool clear_on_paint;
	};
}

#include "camera.hpp"
#include "ui.hpp"