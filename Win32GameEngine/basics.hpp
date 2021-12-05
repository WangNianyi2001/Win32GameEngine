#pragma once

#include <windows.h>
#include <random>
#include <initializer_list>
#include <utility>

namespace Win32GameEngine {
	using String = LPTSTR;
	using ConstString = LPCTSTR;

	template <template <class...> class Template, class... Args>
	void _derived_from_template(const Template<Args...> &);

	template <class T, template <class...> class Template>
	concept derived_from_template = requires(const T & t) {
		_derived_from_template<Template>(t);
	};

	template<typename T, unsigned D>
	struct Vector {
		T data[D];
		inline T at(unsigned i) const { return data[i]; }
		inline T &operator[](unsigned i) { return data[i]; }
		Vector() {}
		template<typename U>
		Vector(Vector<U, D> const &v) {
			for(unsigned i = 0; i < D; ++i)
				operator[](i) = (T)v.at(i);
		}
		Vector<T, D>(std::initializer_list<T> list) {
			T const *arr = list.begin();
			for(unsigned i = 0, m = std::max(list.size(), D); i < m; ++i)
				operator[](i) = arr[i];
		}
		Vector<T, D> operator+(Vector<T, D> v) {
			Vector<T, D> res = *this;
			for(unsigned i = 0; i < D; ++i)
				res[i] += v.at(i);
			return res;
		}
		template<typename S>
		Vector<T, D> operator*(S s) {
			Vector<T, D> res = *this;
			for(unsigned i = 0; i < D; ++i)
				res[i] = (T)(res[i] * s);
			return res;
		}
		Vector<T, D> operator-(Vector<T, D> v) {
			return operator+(v * -1);
		}
	};
	using Vec2I = Vector<int, 2U>;
	using Vec2U = Vector<unsigned, 2U>;
	using Vec2F = Vector<float, 2U>;
	using Vec3F = Vector<float, 3U>;

	struct Transform {
		Vec3F position;
		Vec2F scale;
	};
}