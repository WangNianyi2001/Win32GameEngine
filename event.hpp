#pragma once

#include <map>
#include <set>
#include <functional>

namespace Win32GameEngine {
	using namespace std;

	template <template <class...> class Template, class... Args>
	void _derived_from_template(const Template<Args...> &);

	template <class T, template <class...> class Template>
	concept derived_from_template = requires(const T & t) {
		_derived_from_template<Template>(t);
	};

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

	template<typename Out, typename ...In>
	struct Receiver {
		virtual Out operator()(In ...args) = 0;
	};

	template<typename Out, derived_from_template<Event> Event>
	struct EventReceiver : Receiver<Out, Event> {};

	template<typename Function, typename Out, typename ...In>
	struct Executor : Receiver<Out, In ...> {
		Function f;
		virtual Out operator()(In ...args) override {
			return f(args...);
		}
		Executor(Function f) : f(f) {}
	};

	template<typename Out, typename ...In>
	struct Handler : Executor<function<Out(In ...)>, Out, In ...> {
		using Executor<
			function<Out(In ...)>, Out, In ...
		>::Executor;
	};

	template<typename Next, typename Out, derived_from_template<Event> Event>
	struct EventMedium : EventReceiver<Out, Event> {
		Next const next;
		EventMedium(Next next) : next(next) {}
	};

	template<derived_from_template<Event> Event, typename Out, typename Receiver = Receiver<Out, Event>>
	struct EventDistributor : EventReceiver<Out, Event> {
		using EventType = Event::_Type;
		template<typename T>
		using Container = map<EventType, set<T>>;
		Container<Receiver *> receivers;
		EventDistributor() = default;
		void add(EventType type, Receiver *receiver) {
			receivers[type].insert(receiver);
		}
	};
}