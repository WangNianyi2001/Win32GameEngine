#pragma once

#include "window.hpp"
#include "gameobject.hpp"

namespace Win32GameEngine {
	class Game;			// void <- Game -> Scene
	class Scene;		// Game <- Scene -> void (holds entites in a vector)
	class Entity;		// Entity <- Entity -> Entity (refers to a scene by a pointer)
	class Component;	// Entity <- Component -> void

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

	class Component : public GameObject {
	public:
		Component(Entity *parent) {
			this->parent = (Receiver<GameEvent> *)parent;
		}
	};
}