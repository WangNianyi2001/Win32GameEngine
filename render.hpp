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
		inline bool in(Vec2F p) const {
			return (
				p[0] >= min[0] && p[1] >= min[1] &&
				p[0] <= max[0] && p[1] <= max[1]
			);
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
		inline bool hit(Vec2F uv) const { return bound.in(uv); }
		virtual Color sample(Vec2F uv) const = 0;
		virtual void put(Bitmap &bitmap, Bound bound) = 0;
	};

	class ColorBox : public Texture {
		Bitmap pixel;
		Color color;
	public:
		ColorBox(Entity *entity, Color color, Vec2F size, Vec2F anchor) :
			Texture(entity, size, anchor), color(color), pixel({ 1, 1 }) {
			*pixel.data.get() = color;
			pixel.renewhandle();
			pixel.renewdc();
		}
		ColorBox(Entity *entity, Color color, Vec2F size) : ColorBox(entity, color, size, size * .5f) {}
		inline virtual Color sample(Vec2F uv) const override { return color; }
		virtual void put(Bitmap &dest, Bound bound) override {
			Vec2I pos = bound.topleft(), size = bound.bottomright() - pos;
			AlphaBlend(
				dest.getdc(),
				pos[0], pos[1], size[0], size[1],
				pixel.getdc(),
				0, 0, 1, 1,
				blend_function
			);
		}
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
		virtual void put(Bitmap &dest, Bound bound) override {
			Vec2I pos = bound.topleft(), size = bound.bottomright() - pos;
			AlphaBlend(
				dest.getdc(),
				pos[0], pos[1], size[0], size[1],
				bitmap.getdc(),
				0, 0, bitmap.dimension[0], bitmap.dimension[1],
				blend_function
			);
		}
	};

	class Renderer : public Component {
	protected:
		Bitmap &buffer;
		inline Bitmap &target() {
			return entity->scene->game->window->buffer;
		}
		vector<Entity *> queue;
		Renderer(Entity *entity) : Component(entity),
			queue(),
			clear_on_paint(true),
			buffer(*new Bitmap(target().dimension)),
			order(0)
		{
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
				buffer.renewhandle();
				buffer.renewdc();
				buffer.put(target().getdc());
			});
			add(GameEventType::MOUSEDOWN, [=](GameEvent) {
				Entity *hit = cast(entity->scene->game->mouse.position);
				if(!hit)
					return;
				hit->operator()({ GameEventType::CLICK, Propagation::UP });
			});
		}
		~Renderer() {
			delete &buffer;
		}
		virtual Vec2F screen_texture(Texture const *texture, Vec2F screenp) const = 0;
		virtual Vec2F texture_screen(Texture const *texture, Vec2F texturep) const = 0;
		virtual Vec2F buffer_screen(Vec2I screenp) const = 0;
		virtual Vec2I screen_buffer(Vec2F bufferp) const = 0;
		inline Vec2F buffer_texture(Texture const *texture, Vec2I bufferp) const {
			return screen_texture(texture, buffer_screen(bufferp));
		}
		inline Vec2I texture_buffer(Texture const *texture, Vec2F texturep) const {
			return screen_buffer(texture_screen(texture, texturep));
		}
		virtual bool validate(Entity const *entity) = 0;
		virtual bool compare(Entity const *a, Entity const *b) = 0;
		inline void clear() {
			memset(buffer.data.get(), 0, buffer.size * sizeof(Color));
		}
		virtual void sample() = 0;
	public:
		unsigned order;
		virtual Entity *cast(Vec2F bufferp) {
			for(Entity *target : entity->scene->entities) {
				if(!validate(target))
					continue;
				Texture *texture = target->getcomponent<Texture>();
				Vec2F texturep = buffer_texture(texture, bufferp);
				if(texture->bound.in(texturep))
					return target;
			}
			return nullptr;
		}
		bool clear_on_paint;
	};
}

#include "camera.hpp"
#include "ui.hpp"