#pragma once

// Basic levels of object organization of the game.
// Includes a few classes, see below.

#include "window.hpp"
#include "gameobject.hpp"
#include <algorithm>

namespace Win32GameEngine {
	// The most basic types of game object, listed top-down logically.
	class Game;			// void <- Game -> Scene
	class Scene;		// Game <- Scene -> void (holds entites in a vector)
	class Entity;		// Entity <- Entity -> Entity (refers to a scene by a pointer)
	class Component;	// Entity <- Component -> void

	// The instance of the entire game, communicates with the Win32 API,
	// handles scene management, provides a runtime-long environment for
	// game objects to interact internally with.
	class Game : public GameObject {
	protected:
		Window *const window;
		PAINTSTRUCT *ps = new PAINTSTRUCT{};
		HDC hdc;
	public:
		Game(Window *const w) : window(w) {
			add(GameEventType::UPDATE, [&](GameEvent) {
				window->update();
				add(GameEventType::UPDATE, [&](GameEvent) {
					window->update();
				});
			});
			window->events.add(WM_PAINT, [&](SystemEvent) {
				hdc = BeginPaint(window->handle, ps);
				postpone([&](GameEvent) {
					EndPaint(window->handle, ps);
				});
			});
		}
		void addscene(Scene *scene) {
			children.insert((Receiver<GameEvent> *)scene);
		}
		virtual void init() {
			window->events.add(WM_SYSCOMMAND, [&](SystemEvent event) {
				switch(event.data.wParam) {
				case SC_RESTORE:
					window->restore();
					break;
				case SC_MINIMIZE:
					// Minimizing window is prohibited due to restoring problem
					return 0;
				default:
					event.defaultBehavior();
				}
				return 0;
			});
			window->events.add(WM_QUIT, defaultQuit);
			window->init();
		}
	};

	// Physical separation of entities, I'd say.
	// Different from common frameworks, multiple scenes are
	// allowed to be activate simutaneously.
	class Scene : public GameObject {
	public:
		vector<Entity *> entities;
		Scene(Game *game) {
			parent = (Receiver<GameEvent> *)game;
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
		virtual void propagatedown(GameEvent event) override {
			if(!isactive())
				return;
			for(auto entity : entities)
				((Receiver<GameEvent> *)entity)->operator()(event);
		}
		template<derived_from<Component> Component>
		inline Entity *makeSingluar() {
			return (Entity *)(new Component(new Entity(this)))->parent;
		}
	};

	// Game objects that can exist in a scene, might have children
	// (whose transforms are set relatively to the parent itself).
	// Doesn't have to be substantial (fact is that its substantiality
	// is granted by its components)
	class Entity : public GameObject {
	public:
		Transform transform;
		Scene *scene;
		set<Component *> components;
		Entity(Scene *scene) : scene(scene) {
			scene->addentity(this);
		}
		virtual ~Entity() {
			for(Component *component : components)
				delete (Receiver<GameEvent> *)component;
		}
		virtual void propagateup(GameEvent event) override {
			Receiver<GameEvent>::propagateup(event);
			if(!parent)
				(*scene)(event);
		}
		virtual void propagatedown(GameEvent event) override {
			for(auto component : components)
				((Receiver<GameEvent> *)component)->operator()(event);
			Receiver<GameEvent>::propagatedown(event);
		}
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

	// Abilities an entity have, can have no children.
	class Component : public GameObject {
	public:
		Component(Entity *parent) {
			this->parent = (Receiver<GameEvent> *)parent;
			parent->addcomponent(this);
		}
	};
}