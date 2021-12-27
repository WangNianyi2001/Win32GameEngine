#pragma once

#include "utils.hpp"
#include <map>
#include <set>
#include <functional>

namespace Win32GameEngine {
	using namespace std;

	enum class Propagation {
		NONE = 0, UP = 1, DOWN = 2
	};

	template<typename Type>
	struct Event {
		using _Type = Type;
		Type type;
		Propagation propagation = Propagation::NONE;
		inline bool operator==(Event<Type> e) {
			return type == e.type;
		}
		inline bool operator<(Event<Type> e) {
			return type < e.type;
		}
	};

	template<derived_from_template<Event> Event, typename Ret = void>
	struct Receiver {
		struct Postponed {
			function<void()> action;
			time_t time;
		};
		vector<Postponed> postponeds;
		Receiver() {
			postponeds = vector<Postponed>();
		}
		virtual Ret operator()(Event const &event) = 0;
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

	template<derived_from_template<Event> Event, typename Ret = void>
	struct Handler : Receiver<Event, Ret> {
		using Function = function<Ret(Event const &)>;
		Function f;
		virtual inline Ret operator()(Event const &event) override {
			return f(event);
		}
		Handler(Function f) : f(f) {}
	};

	template<typename Next, derived_from_template<Event> Event>
	struct EventMedium : Receiver<Event> {
		Next const next;
		EventMedium(Next next) : next(next) {}
	};

	template<derived_from_template<Event> Event, typename Ret = void, typename Receiver = Handler<Event, Ret>>
	struct EventDistributor : public Win32GameEngine::Receiver<Event, Ret> {
		using EventType = Event::_Type;
		map<EventType, set<Receiver *>> receivers;
		EventDistributor() : Win32GameEngine::Receiver<Event, Ret>() {}
		~EventDistributor() {
			for(pair<EventType, set<Receiver *>> it : receivers) {
				set<Receiver *> &type = it.second;
				for(Receiver *receiver : type)
					delete receiver;
			}
		}
		virtual Ret miss(Event const &) {
			if constexpr(is_same_v<Ret, void>);
			else
				return Ret();
		}
		inline virtual void propagateup(Event const &event) {}
		inline virtual void propagatedown(Event const &event) {}
		void propagate(Event const &event) {
			switch(event.propagation) {
			case Propagation::UP:
				propagateup(event);
				break;
			case Propagation::DOWN:
				propagatedown(event);
				break;
			}
		}
		virtual Ret operator()(Event const &event) override {
			auto it = receivers.find(event.type);
			if constexpr(is_same_v<Ret, void>) {
				if(it == receivers.end())
					miss(event);
				else {
					for(auto receiver : it->second)
						(*receiver)(event);
				}
				propagate(event);
			} else {
				Ret res;
				if(it == receivers.end())
					res = miss(event);
				else {
					for(auto receiver : it->second)
						res = (*receiver)(event);
				}
				propagate(event);
				return res;
			}
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