#pragma once

#include "basics.h"
#include <map>
#include <set>
#include <functional>

namespace Win32GameEngine {
	using namespace std;

	enum class EventPropagation {
		NONE = 0,
		UP = 1,
		DOWN = 2,
		BOTH = 3
	} propagation;

	template<typename Type, typename Data>
	struct Event {
		using _Type = Type;
		using _Data = Data;
		using _Propagation = EventPropagation;
		Type type;
		EventPropagation propagation;
		Data data;
		inline bool operator==(Event<Type, Data> e) {
			return type == e.type;
		}
		inline bool operator<(Event<Type, Data> e) {
			return type < e.type;
		}
	};

	template<derived_from_template<Event> Event, typename Parent = void, typename Child = void>
	struct Receiver {
		Parent *parent;
		set<Child *> children;
		virtual void operator()(Event event) = 0;
		virtual void propagateup(Event event) {}
		virtual void propagatedown(Event event) {}
		virtual void propagate(Event event) {
			if((int)event.propagation & (int)EventPropagation::UP)
				propagateup(event);
			if((int)event.propagation & (int)EventPropagation::DOWN)
				propagatedown(event);
		}
	};
	// Widow specialization
	template<derived_from_template<Event> Event, derived_from_template<Receiver> Parent>
	struct Receiver<Event, Parent, void> : Receiver<Event, Parent, Receiver<Event>> {
		virtual void propagateup(Event event) override {
			Event up = event;
			up.propagation = EventPropagation::UP;
			(*this->parent)(up);
		}
	};
	// Orphan specialization
	template<derived_from_template<Event> Event, derived_from_template<Receiver> Child>
	struct Receiver<Event, void, Child> : Receiver<Event, Receiver<Event>, Child> {
		virtual void propagatedown(Event event) {
			Event down = event;
			down.propagation = EventPropagation::DOWN;
			for(auto *child : this->children)
				(*child)(down);
		}
	};
	// Full specialization
	template<derived_from_template<Event> Event, derived_from_template<Receiver> Parent, derived_from_template<Receiver> Child>
	struct Receiver<Event, Parent, Child> : Receiver<Event, Parent, Receiver<Event>> {
		using Receiver<Event, Parent, void>::propagateup;
		using Receiver<Event, void, Child>::propagatedown;
	};

	template<derived_from_template<Event> Event>
	struct Handler : Receiver<Event> {
		using Function = function<void(Event)>;
		Function f;
		virtual inline void operator()(Event event) override {
			f(event);
			Receiver<Event>::propagate(event);
		}
		Handler(Function f) : f(f) {}
	};

	template<typename Next, derived_from_template<Event> Event>
	struct EventMedium : Receiver<Event> {
		Next const next;
		EventMedium(Next next) : next(next) {}
	};

	template<derived_from_template<Event> Event, typename Receiver = Handler<Event>>
	struct EventDistributor : Win32GameEngine::Receiver<Event> {
		using EventType = Event::_Type;
		map<EventType, set<Receiver *>> receivers;
		virtual void miss(Event) {}
		virtual void operator()(Event event) override {
			auto it = receivers.find(event.type);
			if(it == receivers.end())
				return miss(event);
			for(auto receiver : it->second)
				(*receiver)(event);
			Win32GameEngine::Receiver<Event>::propagate(event);
		}
		void add(EventType type, Receiver *receiver) {
			auto it = receivers.begin();
			receivers[type].insert(receiver);
		}
		template<typename Action>
		inline void add(EventType type, Action action) {
			add(type, new Receiver(action));
		}
	};
}