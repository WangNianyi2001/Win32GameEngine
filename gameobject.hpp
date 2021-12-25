#pragma once

#include <map>
#include <set>
#include "event.hpp"
#include "basics.hpp"

namespace Win32GameEngine {
	class Game;

	// In-game event types.
	// Construction & destruction event types are omitted since
	// they can be achieved equivalently by constructors & destructors.
	enum class GameEventType {
		UPDATE, PAINT,
		ACTIVATE, INACTIVATE,
	};
	struct GameEvent : Event<GameEventType> {};

	// Abstract class for in-game objects that last long.
	class GameObject : public EventDistributor<GameEvent> {
	private:
		bool active;	// Activation status of a game object. Manipulate activate() and inactivate().
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
}