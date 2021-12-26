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
		UPDATE, PAINT,
		ACTIVATE, INACTIVATE,
	};
	struct GameEvent : Event<GameEventType> {};

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
		inline T *be() const {
			return dynamic_cast<T *>(const_cast<Component *>(this));
		}
	};

	class Entity : public GameObject {
		friend Scene;
	protected:
		Entity(Scene *scene) : scene(scene) {
			add(GameEventType::UPDATE, [&](GameEvent) {
				for(Component *component : components) {
					if(component->isactive())
						component->operator()({ GameEventType::UPDATE });
				}
			});
			add(GameEventType::PAINT, [&](GameEvent) {
				for(Component *component : components) {
					if(component->isactive())
						component->operator()({ GameEventType::PAINT });
				}
			});
		}
		virtual ~Entity() {
			for(Component *component : components)
				delete component;
		}
	public:
		Scene *const scene;
		set<Component *> components;
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
				T *t = component->be<T>();
				if(t)
					return t;
			}
			return nullptr;
		}
	};

	class Scene : public GameObject {
		friend Game;
	protected:
		Scene(Game *game) : GameObject(false), game(game) {
			add(GameEventType::UPDATE, [&](GameEvent) {
				for(Entity *entity : entities) {
					if(entity->isactive())
						entity->operator()({ GameEventType::UPDATE });
				}
			});
			add(GameEventType::PAINT, [&](GameEvent) {
				for(Entity *entity : entities) {
					if(entity->isactive())
						entity->operator()({ GameEventType::PAINT });
				}
			});
		}
		virtual ~Scene() {
			for(Entity *entity : entities)
				delete entity;
		}
	public:
		Game *const game;
		set<Entity *> entities;
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
	protected:
		Ticker time;
	public:
		Window *const window;
		Bitmap buffer;
		set<Scene *> scenes;
		Game(Window *const w) : GameObject(false),
			hdc(nullptr), window(w),
			buffer(window->args.size), time(), frame(0)
		{
			add(GameEventType::UPDATE, [&](GameEvent) { window->update(); });
			window->events.add(WM_PAINT, [&](SystemEvent) {
				hdc = BeginPaint(window->handle, ps);
				postpone([&]() {
					operator()({ GameEventType::PAINT });
				});
				postpone([&]() { EndPaint(window->handle, ps); });
			});
			window->events.add(WM_SYSCOMMAND, [&](SystemEvent event) {
				switch(event.data.wParam) {
				case SC_RESTORE:
					window->restore();
					break;
				case SC_MINIMIZE:
					// Minimizing window is prohibited due to restoring problem
					return;
				default:
					event.defaultBehavior();
				}
			});
			window->events.add(WM_DESTROY, [&](SystemEvent) { inactivate(); });
			window->events.add(WM_QUIT, [&](SystemEvent) { PostQuitMessage(0); });
			add(GameEventType::UPDATE, [&](GameEvent) {
				if(!time.isup())
					return;
				time.tick();
				for(Scene *scene : scenes) {
					if(scene->isactive())
						scene->operator()({ GameEventType::UPDATE });
				}
				if(frame.isup()) {
					repaint();
					frame.tick();
				}
			});
			add(GameEventType::PAINT, [&](GameEvent) {
				for(Scene *scene : scenes) {
					if(scene->isactive())
						scene->operator()({ GameEventType::PAINT });
				}
			});
		}
		Scene *addscene(Scene *scene) {
			scenes.insert(scene);
			return scene;
		}
		inline Scene *makescene() {
			return addscene(new Scene(this));
		}
		inline HDC gethdc() const { return hdc; }
		void repaint() {
			InvalidateRect(window->handle, nullptr, false);
		}
		void setupdaterate(ULONGLONG rate) { time.setrate(rate); }
		void setfps(ULONGLONG fps) { this->frame.setrate(fps ? 1000 / fps : 0); }
	};
}