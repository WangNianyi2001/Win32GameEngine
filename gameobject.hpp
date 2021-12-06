#pragma once

#include <map>
#include "event.hpp"
#include "basics.hpp"

namespace Win32GameEngine {
	enum class GameEventType {
		INIT, KILL,
		UPDATE, FIXEDUPDATE,
		ACTIVATE, INACTIVATE
	};
	struct GameEventData {};
	struct GameEvent : Event<GameEventType, GameEventData> {};

	template<typename Parent = void, typename Child = void>
	class GameObject : public EventDistributor<GameEvent, Handler<GameEvent>, Parent, Child> {
	private:
		bool active = true;
	public:
		virtual void oninit() {}
		virtual void onkill() {}
		virtual void onupdate() {}
		virtual void onfixedupdate() {}
		virtual void onactivate() {}
		virtual void oninactivate() {}
		GameObject<Parent, Child>() {
			this->add(GameEventType::INIT, [&](GameEvent) { oninit(); });
			this->add(GameEventType::KILL, [&](GameEvent) { onkill(); });
			this->add(GameEventType::UPDATE, [&](GameEvent) { onupdate(); });
			this->add(GameEventType::FIXEDUPDATE, [&](GameEvent) { onfixedupdate(); });
			this->add(GameEventType::ACTIVATE, [&](GameEvent) { onactivate(); });
			this->add(GameEventType::INACTIVATE, [&](GameEvent) { oninactivate(); });
		}
		inline bool isactive() { return active; }
		inline void activate() {
			if(active)
				return;
			active = true;
			onactivate();
		}
		inline void inactivate() {
			if(!active)
				return;
			active = false;
			oninactivate();
		}
	};

	class Scene;
	class Entity;

	class Component : public GameObject<Entity> {
		//
	};

	class Entity : public GameObject<Entity, Entity> {
	public:
		Transform transform;
		Scene *const scene;
		set<Component *> components;
		Entity(Scene *const scene) : scene(scene) {}
		virtual void propagatedown(GameEvent event) override {
			for(Component *component : components)
				component->operator()(event);
			GameObject::propagatedown(event);
		}
		template<derived_from<Component> C>
		void addComponent(C *component) {
			components.insert(component);
		}
	};
}