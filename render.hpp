#pragma once

#include "game.hpp"
#include "transform.hpp"

namespace Win32GameEngine {
	struct RectBound {
		using V = Vec2F;
		V min, max;
		RectBound() : min{ INFINITY, INFINITY }, max{ -INFINITY, -INFINITY } {}
		RectBound(V min, V max) : RectBound() {
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
		RectBound transform(function<V(V)> f) const {
			RectBound res;
			res.add(f(topleft()));
			res.add(f(topright()));
			res.add(f(bottomleft()));
			res.add(f(bottomright()));
			return res;
		}
		RectBound transform(SquareMatrix<3, float> m) const {
			return transform([&](Vec2F v) { return m(v); });
		}
		RectBound clip(RectBound r) const {
			RectBound res;
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
		RectBound bound;
		Texture(Entity *entity, Vec2F size, Vec2F anchor) :
			Component(entity), size(size), anchor(anchor),
			bound(RectBound(anchor * -1, size - anchor)) {
		}
		void setanchor(Vec2F a) {
			anchor = a;
			bound = RectBound(a * -1, size - a);
		}
		virtual bool hit(Vec2F uv) const {
			float x = uv[0], y = uv[1];
			return (x < bound.max[0]) && (x >= bound.min[0]) && (y < bound.max[1]) && (y >= bound.min[1]);
		}
		virtual Color sample(Vec2F uv) const = 0;
		virtual void put(HDC hdc, RectBound bound) = 0;
	};

	class ColorBox : public Texture {
	public:
		Color color;
		ColorBox(Entity *entity, Color color, Vec2F size, Vec2F anchor) :
			Texture(entity, size, anchor), color(color) {
		}
		ColorBox(Entity *entity, Color color, Vec2F size) : ColorBox(entity, color, size, size * .5f) {}
		inline virtual Color sample(Vec2F uv) const override { return color; }
		virtual void put(HDC hdc, RectBound bound) override {
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
		virtual void put(HDC hdc, RectBound bound) override {
			Vec2F pos_dest = bound.topleft(), size_dest = bound.bottomright() - pos_dest;
			AlphaBlend(
				hdc,
				pos_dest[0], pos_dest[1], size_dest[0], size_dest[1],
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
		bool clear_on_paint;
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
		inline void setclearonpaint(bool b) {
			clear_on_paint = b;
		}
	};
}

#include "camera.hpp"
#include "ui.hpp"