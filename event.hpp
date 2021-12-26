#pragma once

// Interface for the event systems used across the SDK.

#include "utils.hpp"
#include <map>
#include <set>
#include <functional>

namespace Win32GameEngine {
	using namespace std;

	// Basic structure for a single event, templated.
	// Type is to identify the class of the event, e.g. "update" or "click".
	template<typename Type>
	struct Event {
		using _Type = Type;
		Type type;
		inline bool operator==(Event<Type> e) {
			return type == e.type;
		}
		inline bool operator<(Event<Type> e) {
			return type < e.type;
		}
	};

	template<derived_from_template<Event> Event>
	struct Receiver {
		struct Postponed {
			function<void()> action;
			time_t time;
		};
		vector<Postponed> postponeds;
		Receiver() {
			postponeds = vector<Postponed>();
		}
		virtual void operator()(Event event) = 0;
		void postpone(function<void()> action, time_t time = 0) {
			Postponed postponed{ action, time };
			postponeds.push_back(postponed);
		}
		void resolve() {
			for(auto it = postponeds.begin(); it != postponeds.end(); ) {
				Postponed postponed = *it;
				if(postponed.time) {	// TODO: time check
					++it;
					continue;
				}
				it = postponeds.erase(it);
				postponed.action();
			}
		}
	};

	template<derived_from_template<Event> Event>
	struct Handler : Receiver<Event> {
		using Function = function<void(Event)>;
		Function f;
		virtual inline void operator()(Event event) override {
			f(event);
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
		EventDistributor() : Win32GameEngine::Receiver<Event>() {}
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