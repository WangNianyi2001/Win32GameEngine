#pragma once

#include <algorithm>
#include <set>
#include "utils.hpp"

namespace Win32GameEngine {
	class Game;
	class Scene;
	class Entity;
	class Component;

	enum class GameEventType {
		INIT, QUIT,
		UPDATE, POSTUPDATE,
		PAINT, POSTPAINT,
		MOUSEDOWN, MOUSEUP, CLICK,
		ACTIVATE, INACTIVATE,
	};
	struct GameEvent : Event<GameEventType> {
	};

	class GameObject : public EventDistributor<GameEvent> {
	private:
		bool active;
	public:
		GameObject(bool active = true) : active(false) {
			if(active)
				activate();
		}
		inline bool isactive() const { return active; }
		inline void activate() {
			if(active)
				return;
			active = true;
			operator()({ GameEventType::ACTIVATE });
		}
		inline void inactivate() {
			if(!active)
				return;
			active = false;
			operator()({ GameEventType::INACTIVATE });
		}
	};

	class Component : public GameObject {
		friend Entity;
	protected:
		Component(Entity *entity) : entity(entity) {}
	public:
		Entity *const entity;
		template<derived_from<Component> T>
		inline T *as() const {
			return dynamic_cast<T *>(const_cast<Component *>(this));
		}
	};

	class Entity : public GameObject {
		friend Scene;
	protected:
		Entity(Scene *scene) : scene(scene) {}
		virtual ~Entity() {
			for(Component *component : components)
				delete component;
		}
	public:
		Scene *const scene;
		set<Component *> components;
		virtual void propagatedown(GameEvent event) override {
			for(Component *component : components) {
				if(component->isactive())
					component->operator()(event);
			}
		}
		Component *addcomponent(Component *component) {
			components.insert(component);
			return component;
		}
		template<derived_from<Component> Component, typename ...Args>
		inline Component *makecomponent(Args ...args) {
			return (Component *)addcomponent(new Component(this, args...));
		}
		template<derived_from<Component> T>
		T *getcomponent() const {
			for(Component *component : components) {
				T *t = component->as<T>();
				if(t)
					return t;
			}
			return nullptr;
		}
	};

	class Scene : public GameObject {
		friend Game;
	protected:
		Scene(Game *game) : GameObject(false), game(game) {}
		virtual ~Scene() {
			for(Entity *entity : entities)
				delete entity;
		}
	public:
		Game *const game;
		set<Entity *> entities;
		virtual void propagatedown(GameEvent event) override {
			for(Entity *entity : entities) {
				if(entity->isactive())
					entity->operator()(event);
			}
		}
		Entity *addentity(Entity *entity) {
			entities.insert(entity);
			return entity;
		}
		inline Entity *makeentity() {
			return addentity(new Entity(this));
		}
	};

	class Game : public GameObject {
	private:
		Ticker frame;
		PAINTSTRUCT *ps = new PAINTSTRUCT{};
		HDC hdc;
	public:
		Window *window;
		HINSTANCE hinst;
		bool clear_frame_buffer;
		set<Scene *> scenes;
		Ticker time;
		Game() : GameObject(false),
			hdc(nullptr), window(nullptr),
			clear_frame_buffer(true),
			time(), frame(0)
		{}
		void start() {
			operator()({ GameEventType::INIT });

			// System events redirection
			for(UINT type : vector<UINT>{ WM_LBUTTONDOWN, WM_MBUTTONDOWN, WM_RBUTTONDOWN }) {
				window->events.add(type, [&](SystemEvent event) {
					operator()({ GameEventType::MOUSEDOWN, Propagation::DOWN });
					return event.def();
				});
			}
			for(UINT type : vector<UINT>{ WM_LBUTTONUP, WM_MBUTTONUP, WM_RBUTTONUP }) {
				window->events.add(type, [=](SystemEvent event) {
					operator()({ GameEventType::MOUSEUP, Propagation::DOWN });
					return event.def();
				});
			}
			window->events.add(WM_PAINT, [&](SystemEvent event) {
				operator()({ GameEventType::PAINT, Propagation::DOWN });
				operator()({ GameEventType::POSTPAINT, Propagation::DOWN });
				return event.def();
			});
			window->events.add(WM_SYSCOMMAND, [&](SystemEvent event) {
				switch(event.data.w) {
				case SC_MAXIMIZE:
					return (LRESULT)0;
				case SC_RESTORE:
					window->restore();
					break;
				}
				return event.def();
			});
			window->events.add(WM_CLOSE, [&](SystemEvent event) {
				operator()({ GameEventType::QUIT, Propagation::DOWN });
				return event.def();
			});
			window->events.add(WM_DESTROY, [&](SystemEvent event) {
				PostQuitMessage(0);
				return event.def();
			});

			// In-game events logic
			add(GameEventType::UPDATE, [&](GameEvent) {
				window->update();
				if(!time.isup())
					return;
				time.tick();
			});
			add(GameEventType::POSTUPDATE, [&](GameEvent) {
				if(frame.isup()) {
					repaint();
					frame.tick();
				}
			});
			add(GameEventType::PAINT, [&](GameEvent) {
				if(clear_frame_buffer) {
					HDC bdc = window->buffer.getdc();
					SelectObject(bdc, GetStockObject(NULL_PEN));
					SelectObject(bdc, window->args.background_brush);
					Vec2I s = window->args.size;
					Rectangle(bdc, 0, 0, s[0], s[1]);
				}
				hdc = BeginPaint(window->handle, ps);
			});
			add(GameEventType::POSTPAINT, [&](GameEvent) {
				Vec2I s = window->buffer.dimension;
				BitBlt(hdc, 0, 0, s[0], s[1], window->buffer.getdc(), 0, 0, SRCCOPY);
				EndPaint(window->handle, ps);
			});

			activate();
		}
		virtual void propagatedown(GameEvent event) override {
			for(Scene *scene : scenes) {
				if(scene->isactive())
					scene->operator()(event);
			}
		}
		Scene *addscene(Scene *scene) {
			scenes.insert(scene);
			return scene;
		}
		inline Scene *makescene() {
			return addscene(new Scene(this));
		}
		void update() {
			resolve();
			operator()({ GameEventType::UPDATE, Propagation::DOWN });
			operator()({ GameEventType::POSTUPDATE, Propagation::DOWN });
		}
		void repaint() {
			InvalidateRect(window->handle, nullptr, true);
		}
		void setupdaterate(ULONGLONG rate) { time.setrate(rate); }
		void setfps(ULONGLONG fps) { this->frame.setrate(fps ? 1000 / fps : 0); }
	};
}

#include "transform.hpp"
#include "render.hpp"