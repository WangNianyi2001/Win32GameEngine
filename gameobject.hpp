#pragma once

#include <map>
#include "event.hpp"
#include "basics.hpp"

namespace Win32GameEngine {
	enum class GameEventType {
		UPDATE, POSTUPDATE, FIXEDUPDATE,
		PAINT,
		ACTIVATE, INACTIVATE
	};
	struct GameEventData {};
	struct GameEvent : Event<GameEventType, GameEventData> {};

	class GameObject : public EventDistributor<GameEvent, Handler<GameEvent>> {
	private:
		bool active = true;
	public:
		virtual void onupdate() {}
		virtual void onpostupdate() {}
		virtual void onfixedupdate() {}
		virtual void onpaint() {}
		virtual void onactivate() {}
		virtual void oninactivate() {}
		virtual void onkill() {}
		GameObject() {
			this->add(GameEventType::UPDATE, [&](GameEvent) { onupdate(); });
			this->add(GameEventType::POSTUPDATE, [&](GameEvent) { onpostupdate(); });
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