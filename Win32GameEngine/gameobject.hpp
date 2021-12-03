#pragma once

#include <initializer_list>
#include <map>
#include <functional>
#include "event.hpp"

namespace Win32GameEngine {
	enum class GameEventType {
		INIT, KILL,
		UPDATE, FIXEDUPDATE,
		ACTIVATE, INACTIVATE
	};
	struct GameEventData {};
	using GameEvent = Event<GameEventType, GameEventData>;

	template<typename Parent = void, typename Child = void>
	class GameObject : public EventDistributor<GameEvent, Handler<GameEvent>, Parent, Child> {
	private:
		bool active = true;
	public:
		inline bool isactive() { return active; }
		inline void setactivity(bool a) { active = a; }
		virtual void oninit() {}
		virtual void onkill() {}
		virtual void onupdate() {}
		virtual void onfixedupdate() {}
		virtual void onactivate() {}
		virtual void oninactivate() {}
		GameObject<Parent, Child>() {
			this->add(GameEventType::INIT, [&](GameEvent) { oninit(); });
			this->add(GameEventType::KILL, [&](GameEvent) { onkill(); });
			this->add(GameEventType::UPDATE, [&](GameEvent) { onupdate(); });
			this->add(GameEventType::FIXEDUPDATE, [&](GameEvent) { onfixedupdate(); });
			this->add(GameEventType::ACTIVATE, [&](GameEvent) { onactivate(); });
			this->add(GameEventType::INACTIVATE, [&](GameEvent) { oninactivate(); });
		}
	};

	template<typename T, unsigned D>
	struct Vector {
		T data[D];
		inline T at(unsigned i) const { return data[i]; }
		inline T &operator[](unsigned i) { return data[i]; }
		Vector<T, D>(Vector<T, D> const &v) {
			for(unsigned i = 0; i < D; ++i)
				operator[](i) = v.at(i);
		}
		Vector<T, D>(std::initializer_list<T> list) {
			T const *arr = list.begin();
			for(unsigned i = 0, m = std::max(list.size(), D); i < m; ++i)
				operator[](i) = arr[i];
		}
		Vector<T, D>() : Vector<T, D>{ 0, 0, 0 } {}
		Vector<T, D> operator+(Vector<T, D> v) {
			Vector<T, D> res = *this;
			for(unsigned i = 0; i < D; ++i)
				res[i] += v.at(i);
			return res;
		}
		Vector<T, D> operator*(T s) {
			Vector<T, D> res = *this;
			for(unsigned i = 0; i < D; ++i)
				res[i] *= s;
			return res;
		}
		Vector<T, D> operator-(Vector<T, D> v) {
			return operator+(v * -1);
		}
	};
	using Vec2F = Vector<float, 2U>;
	using Vec3F = Vector<float, 3U>;

	class Entity : public GameObject<Entity, Entity> {
	public:
		struct Transform {
			Vec3F position;
			Vec2F scale;
		};
		Transform transform;
	};
}