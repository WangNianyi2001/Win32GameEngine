#pragma once

// Interface for the event systems used across the SDK.

#include "basics.hpp"
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

	// Basic structure for a single event, templated.
	// Type is to identify the class of the event, e.g. "update" or "click".
	// Propagation defines how will the event propagates across parent & children.
	// Data is a custom structure storing the information that come with the event.
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

	template<derived_from_template<Event> Event>
	struct Receiver {
		Receiver *parent;
		set<Receiver *> children;
		Receiver(Receiver *parent = nullptr) : parent(parent) {}
		virtual void operator()(Event event) = 0;
		virtual void propagateup(Event event) {
			Event up = event;
			up.propagation = EventPropagation::UP;
			if(parent)
				(*parent)(up);
		}
		virtual void propagatedown(Event event) {
			Event down = event;
			down.propagation = EventPropagation::DOWN;
			for(auto *child : children)
				(*child)(down);
		}
		void propagate(Event event) {
			if((int)event.propagation & (int)EventPropagation::UP)
				propagateup(event);
			if((int)event.propagation & (int)EventPropagation::DOWN)
				propagatedown(event);
		}
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
	struct EventDistributor : public Win32GameEngine::Receiver<Event> {
		using EventType = Event::_Type;
		map<EventType, set<Receiver *>> receivers;
		EventDistributor(Receiver *parent = nullptr) : Win32GameEngine::Receiver<Event>(parent) {}
		~EventDistributor() {
			for(pair<EventType, set<Receiver *>> it : receivers) {
				set<Receiver *> &type = it.second;
				for(Receiver *receiver : type)
					delete receiver;
			}
		}
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