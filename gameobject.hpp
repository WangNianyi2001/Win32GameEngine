#pragma once

#include <map>
#include "event.hpp"
#include "basics.hpp"

namespace Win32GameEngine {
	class Game;

	// In-game event types.
	// Construction & destruction event types are omitted since
	// they can be achieved equivalently by constructors & destructors.
	enum class GameEventType {
		UPDATE, FIXEDUPDATE,
		PAINT,
		ACTIVATE, INACTIVATE
	};
	// Temporarily set empty. Might be fulfilled in the future.
	struct GameEventData {};
	struct GameEvent : Event<GameEventType, GameEventData> {};

	// Abstract class for in-game objects that last long.
	// Each game object has its own (possible) parent and children.
	// When events occur, they might propagate along the parent & children.
	class GameObject : public EventDistributor<GameEvent, Handler<GameEvent>> {
	private:
		bool active = true;	// Activation status of a game object. Manipulate activate() and inactivate().
	public:
		virtual void onupdate() {}
		virtual void onfixedupdate() {}
		virtual void onpaint() {}
		virtual void onactivate() {}
		virtual void oninactivate() {}
		virtual void onkill() {}
		GameObject() {
			// Bind events at construction for Unity-style event management.
			// Event mediums might be applied in the future.
			this->add(GameEventType::UPDATE, [&](GameEvent) { onupdate(); });
			this->add(GameEventType::FIXEDUPDATE, [&](GameEvent) { onfixedupdate(); });
			this->add(GameEventType::PAINT, [&](GameEvent) { onpaint(); });
			this->add(GameEventType::ACTIVATE, [&](GameEvent) { onactivate(); });
			this->add(GameEventType::INACTIVATE, [&](GameEvent) { oninactivate(); });
		}
		virtual ~GameObject() {
			for(auto child : children)
				delete child;
		}
		inline bool isactive() { return active; }
		inline void activate() {
			if(active)
				return;
			active = true;
			operator()({ GameEventType::ACTIVATE, EventPropagation::DOWN });
		}
		inline void inactivate() {
			if(!active)
				return;
			active = false;
			operator()({ GameEventType::INACTIVATE, EventPropagation::DOWN });
		}
	};
}