#pragma once

// Basic levels of object organization of the game.
// Includes a few classes, see below.

#include "window.hpp"
#include "gameobject.hpp"
#include <algorithm>

namespace Win32GameEngine {
	// The most basic types of game object, listed top-down logically.
	class Game;
	class Scene;
	class Entity;
	class Component;

	// Components are the abilities an entity have.
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

	// Game objects that can exist in a scene.
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
	public:
		Scene *const scene;
		set<Component *> components;
		Entity *parent;
		virtual ~Entity() {
			for(Component *component : components)
				delete component;
		}
		void addcomponent(Component *component) { components.insert(component); }
		template<derived_from<Component> Component, typename ...Args>
		inline Component *makecomponent(Args ...args) {
			Component *component = new Component(this, args...);
			addcomponent(component);
			return component;
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

	// Physical separation of entities.
	class Scene : public GameObject {
		friend Game;
	protected:
		Scene(Game *game) : game(game) {
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
	public:
		Game *const game;
		set<Entity *> entities;
		vector<Entity *> solid_entities;
		virtual ~Scene() {
			for(Entity *entity : entities)
				delete entity;
		}
		inline void addentity(Entity *entity) { entities.insert(entity); }
		inline Entity *makeentity() {
			Entity *entity = new Entity(this);
			addentity(entity);
			return entity;
		}
		template<derived_from<Component> Component, typename ...Args>
		Entity *makeentity(Args ...args) {
			Entity *entity = makeentity();
			entity->makecomponent<Component>(args...);
			return entity;
		}
	};

	// The instance of the entire game, communicates with the Win32 API,
	// handles scene management, provides a runtime-long environment for
	// game objects to interact internally with.
	class Game : public GameObject {
	protected:
		Window *const window;
		PAINTSTRUCT *ps = new PAINTSTRUCT{};
		HDC hdc;
		ULONGLONG const start_tick;
		ULONGLONG last_tick;
	public:
		set<Scene *> scenes;
		Game(Window *const w) : window(w), start_tick(gettick()), last_tick(start_tick) {
			add(GameEventType::UPDATE, [&](GameEvent) { window->update(); });
			window->events.add(WM_PAINT, [&](SystemEvent) {
				hdc = BeginPaint(window->handle, ps);
				postpone([&]() { operator()({ GameEventType::PAINT }); });
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
				for(Scene *scene : scenes) {
					if(scene->isactive())
						scene->operator()({ GameEventType::UPDATE });
				}
				last_tick = gettick();
			});
			add(GameEventType::PAINT, [&](GameEvent) {
				for(Scene *scene : scenes) {
					if(scene->isactive())
						scene->operator()({ GameEventType::PAINT });
				}
			});
		}
		void addscene(Scene *scene) { scenes.insert(scene); }
		inline Scene *makescene() {
			Scene *scene = new Scene(this);
			addscene(scene);
			return scene;
		}
		inline HDC gethdc() const { return hdc; }
		static inline ULONGLONG gettick() { return GetTickCount64(); }
		inline ULONGLONG elapsedtick() { return gettick() - start_tick; }
		inline ULONGLONG frametick() { return gettick() - last_tick; }
	};
}