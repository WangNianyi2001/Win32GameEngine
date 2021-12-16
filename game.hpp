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

	// The instance of the entire game, communicates with the Win32 API,
	// handles scene management, provides a runtime-long environment for
	// game objects to interact internally with.
	class Game : public GameObject {
	protected:
		Window *const window;
		PAINTSTRUCT *ps = new PAINTSTRUCT{};
		HDC hdc;
	public:
		set<Scene *> scenes;
		Game(Window *const w) : window(w) {
			add(GameEventType::UPDATE, [&](GameEvent) { window->update(); });
			window->events.add(WM_PAINT, [&](SystemEvent) {
				hdc = BeginPaint(window->handle, ps);
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
		}
		void addscene(Scene *scene) {
			scenes.insert(scene);
		}
	};

	// Physical separation of entities.
	class Scene : public GameObject {
	public:
		Game *game;
		vector<Entity *> entities;
		Scene(Game *game) : game(game) {
			game->addscene(this);
			// Sort by Z coordinate when update
			add(GameEventType::UPDATE, [&](GameEvent) {
				sort(entities.begin(), entities.end());
			});
		}
		virtual ~Scene() {
			for(Entity *entity : entities)
				delete (Receiver<GameEvent> *)entity;
		}
		void addentity(Entity *entity) {
			// Insert the new entity to the correct Z-ordered place.
			auto it = entities.begin();
			for(auto end = entities.end(); it != end; ++it) {
				if(entity < *it)
					break;
			}
			entities.insert(it, entity);
		}
		template<derived_from<Component> Component>
		inline Entity *makesingular() {
			return (new Component(new Entity(this)))->entity;
		}
	};

	// Game objects that can exist in a scene.
	class Entity : public GameObject {
	public:
		Scene *scene;
		set<Component *> components;
		Entity *parent;
		set<Entity *>children;
		Transform transform;
		Entity(Scene *scene) : scene(scene) {
			scene->addentity(this);
		}
		virtual ~Entity() {
			for(Component *component : components)
				delete (Receiver<GameEvent> *)component;
		}
		// Less in Z coordinate
		bool operator<(Entity const &entity) const {
			return transform.position.at(2) < entity.transform.position.at(2);
		}
		void setparent(Entity *entity) {
			if(parent)
				parent->children.erase(this);
			if(parent = entity)
				parent->children.insert(this);
		}
		void addcomponent(Component *component) {
			components.insert(component);
		}
	};

	// Components are the abilities an entity have.
	class Component : public GameObject {
	public:
		Entity *entity;
		Component(Entity *entity) : entity(entity) {
			entity->addcomponent(this);
		}
	};
}