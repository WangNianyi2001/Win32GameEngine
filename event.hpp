#pragma once

#include "basics.h"
#include <map>
#include <set>
#include <functional>

namespace Win32GameEngine {
	using namespace std;

	enum class EventPropragation {
		DISABLED, UP, DOWN
	};
	template<typename Type, typename Data>
	struct Event {
		using _Type = Type;
		Type type;
		EventPropragation propagation;
		Data data;
		inline bool operator==(Event<Type, Data> e) {
			return type == e.type;
		}
		inline bool operator<(Event<Type, Data> e) {
			return type < e.type;
		}
	};

	template<typename ...In>
	struct Receiver {
		virtual void operator()(In ...args) = 0;
	};

	template<typename ...In>
	struct Handler : Receiver<In ...> {
		using Function = function<void(In ...)>;
		Function f;
		virtual inline void operator()(In ...args) override {
			f(args...);
		}
		Handler(Function f) : f(f) {}
	};

	template<typename Next, derived_from_template<Event> Event>
	struct EventMedium : Receiver<Event> {
		Next const next;
		EventMedium(Next next) : next(next) {}
	};

	template<derived_from_template<Event> Event, typename _Receiver = Handler<Event>>
	struct EventDistributor : Receiver<Event> {
		using EventType = Event::_Type;
		template<typename T>
		using Container = map<EventType, set<T>>;
		Container<_Receiver *> receivers;
		EventDistributor() = default;
		virtual void miss(Event) {}
		virtual void operator()(Event event) override {
			auto it = receivers.find(event.type);
			if(it == receivers.end())
				return miss(event);
			for(auto receiver : it->second)
				receiver->operator()(event);
		}
		void add(EventType type, _Receiver *receiver) {
			receivers[type].insert(receiver);
		}
		template<typename Function>
		void add(EventType type, Function receiver) {
			receivers[type].insert(new _Receiver(receiver));
		}
	};
}