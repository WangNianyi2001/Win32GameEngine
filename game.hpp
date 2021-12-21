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
		Entity const *entity;
		Component(Entity *entity) : entity(entity) {}
	public:
	};

	// Game objects that can exist in a scene.
	class Entity : public GameObject {
		friend Scene;
	protected:
		Scene const *scene;
		Entity(Scene *scene) : scene(scene) {}
	public:
		struct Transform : AffineMatrix<3, float> {
			template<typename T>
			struct Attribute {
			protected:
				T value;
				Transform *const transform;
			public:
				Attribute(Transform *t, T const &v) : transform(t) {
					operator=(v);
				}
				inline T operator()() { return value; }
				Attribute<T> &operator=(T const &v) {
					value = v;
					transform->update();
					return *this;
				}
			};
			Attribute<Vec3F> position;
			Attribute<float> rotation;
			Attribute<Vec3F> scale;
			Transform() : position(this, { 0, 0, 0 }), rotation(this, .0f), scale(this, { 1, 1, 1 }) {
				row(2)[2] = 1;
			}
		protected:
			friend Attribute<Vec3F>;
			friend Attribute<float>;
			void update() {
				float rot = rotation();
				Vec3F sca = scale();
				float c = cos(rot), s = sin(rot);
				float x = sca[0], y = sca[1];
				row(0) = Vec2F{ x * c, -y * s };
				row(1) = Vec2F{ x * s, y * c };
				col(3) = position();
			}
		} transform;
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
	};

	// Physical separation of entities.
	class Scene : public GameObject {
		friend Game;
	protected:
		Game const *game;
		Scene(Game *game) : game(game) {
			// Sort by Z coordinate when update
			add(GameEventType::UPDATE, [&](GameEvent) {
				sort(entities.begin(), entities.end());
			});
		}
	public:
		vector<Entity *> entities;
		virtual ~Scene() {
			for(Entity *entity : entities)
				delete entity;
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
		inline Entity *makeentity() {
			Entity *entity = new Entity(this);
			addentity(entity);
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
		void addscene(Scene *scene) { scenes.insert(scene); }
		inline Scene *makescene() {
			Scene *scene = new Scene(this);
			addscene(scene);
			return scene;
		}
	};
}