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
				for(Component *component : components)
					component->operator()({ GameEventType::UPDATE });
			});
			add(GameEventType::FIXEDUPDATE, [&](GameEvent) {
				for(Component *component : components)
					component->operator()({ GameEventType::FIXEDUPDATE });
			});
			add(GameEventType::PAINT, [&](GameEvent) {
				for(Component *component : components)
					component->operator()({ GameEventType::PAINT });
			});
		}
	public:
		Scene *const scene;
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
			AffineMatrix<3, float> inverse;
		protected:
			friend Attribute<Vec3F>;
			friend Attribute<float>;
			void update() {
				float rot = rotation();
				Vec3F sca = scale();
				float c = cos(rot), s = sin(rot);
				float x = 1 / sca[0], y = 1 / sca[1];
				row(0) = Vec2F{ x * c, -y * s };
				row(1) = Vec2F{ x * s, y * c };
				col(3) = position();
				inverse = AffineMatrix<3, float>::inverse();
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
				// Sort entities by Z coordinate
				sort(entities.begin(), entities.end());
				for(Entity *entity : entities)
					entity->operator()({ GameEventType::UPDATE });
			});
			add(GameEventType::FIXEDUPDATE, [&](GameEvent) {
				for(Entity *entity : entities)
					entity->operator()({ GameEventType::FIXEDUPDATE });
			});
			add(GameEventType::PAINT, [&](GameEvent) {
				for(Entity *entity : entities)
					entity->operator()({ GameEventType::PAINT });
			});
		}
	public:
		Game *const game;
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
	public:
		set<Scene *> scenes;
		Game(Window *const w) : window(w) {
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
				for(Scene *scene : scenes)
					scene->operator()({ GameEventType::UPDATE });
			});
			add(GameEventType::FIXEDUPDATE, [&](GameEvent) {
				for(Scene *scene : scenes)
					scene->operator()({ GameEventType::FIXEDUPDATE });
			});
			add(GameEventType::PAINT, [&](GameEvent) {
				for(Scene *scene : scenes)
					scene->operator()({ GameEventType::PAINT });
			});
		}
		void addscene(Scene *scene) { scenes.insert(scene); }
		inline Scene *makescene() {
			Scene *scene = new Scene(this);
			addscene(scene);
			return scene;
		}
		inline HDC gethdc() const { return hdc; }
	};
}