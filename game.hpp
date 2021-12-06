#pragma once

// Basic levels of object organization of the game.
// Includes a few classes, see below.

#include "window.hpp"
#include "gameobject.hpp"

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
	public:
		Game(Window *const w) : window(w) {
			add(GameEventType::UPDATE, [&](GameEvent) {
				window->update();
			});
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
		}
		void addentity(Entity *entity) {
			entities.push_back(entity);
		}
		virtual void propagatedown(GameEvent event) override {
			if(!isactive())
				return;
			Receiver<GameEvent>::propagatedown(event);
		}
		virtual ~Scene() {
			for(auto entity : entities)
				delete entity;
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
		Entity(Scene *scene) : scene(scene) {}
		virtual ~Entity() {
			for(auto component : components)
				delete component;
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
		}
	};
}