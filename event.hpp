#pragma once

#include <map>
#include <set>
#include <functional>

namespace Win32GameEngine {
	using namespace std;

	using RawEvent = UINT;

	template<typename Out, typename ...In>
	struct EventReceiver {
		virtual Out operator()(In ...args) = 0;
	};

	template<typename Function, typename Out, typename ...In>
	struct EventReceiverWrapper : EventReceiver<Out, In ...> {
		Function f;
		virtual Out operator()(In ...args) {
			return f(args...);
		}
		EventReceiverWrapper(Function f) : f(f) {}
	};

	template<typename Out, typename ...In>
	struct Handler : EventReceiverWrapper<function<Out(In ...)>, Out, In ...> {
		using EventReceiverWrapper<
			function<Out(In ...)>, Out, In ...
		>::EventReceiverWrapper;
	};

	template<typename Next, typename Out, typename ...In>
	struct EventMedium : EventReceiver<Out, In ...> {
		Next const next;
		virtual Out operator()(In ...args) = 0;
		EventMedium(Next next) : next(next) {}
	};

	template<typename Event, typename Receiver, typename Out, typename ...In>
	struct EventDistributor : EventReceiver<Out, In ...> {
		template<typename T>
		using Container = map<Event, set<T>>;
		Container<Receiver *> receivers;
		EventDistributor() = default;
		virtual Out operator()(In ...) = 0;
		void add(Event type, Receiver *receiver) {
			receivers[type].insert(receiver);
		}
	};
}