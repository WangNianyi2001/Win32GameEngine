#pragma once

#include <windows.h>
#include <initializer_list>
#include <utility>

namespace Win32GameEngine {
	using namespace std;

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
		inline T const &operator[](unsigned i) const { return data[i]; }
		Vector() {}
		template<typename U>
		Vector(Vector<U, D> const &v) {
			for(unsigned i = 0; i < D; ++i)
				operator[](i) = (T)v.at(i);
		}
		Vector(std::initializer_list<T> list) {
			T const *arr = list.begin();
			for(unsigned i = 0, m = std::max(list.size(), D); i < m; ++i)
				operator[](i) = arr[i];
		}
		Vector<T, D> operator+(Vector<T, D> v) const {
			Vector<T, D> res;
			for(unsigned i = 0; i < D; ++i)
				res[i] = at(i) + v.at(i);
			return res;
		}
		template<typename S>
		Vector<T, D> operator*(S s) const {
			Vector<T, D> res;
			for(unsigned i = 0; i < D; ++i)
				res[i] = (T)(at(i) * s);
			return res;
		}
		Vector<T, D> operator-(Vector<T, D> v) const {
			return operator+(v * -1);
		}
		Vector<T, D> scale(Vector<T, D> v) const {
			Vector<T, D> res;
			for(unsigned i = 0; i < D; ++i)
				res[i] = at(i) * v.at(i);
			return res;
		}
		Vector<T, D> inverse() const {
			Vector<T, D> res;
			for(unsigned i = 0; i < D; ++i)
				res[i] = (T)1 / at(i);
			return res;
		}
		bool operator==(Vector<T, D> v) const {
			for(unsigned i = 0; i < D; ++i) {
				if(at(i) != v.at(i))
					return false;
			}
			return true;
		}
		bool operator!=(Vector<T, D> v) const {
			return !operator==(v);
		}
	};
	using Vec2I = Vector<int, 2U>;
	using Vec2U = Vector<unsigned, 2U>;
	using Vec2F = Vector<float, 2U>;
	using Vec3F = Vector<float, 3U>;

	struct Transform {
		Vec3F position;
		Vec3F scale;
		inline Transform operator*(Transform const &transform) const {
			return {
				position.scale(transform.scale) + transform.position,
				scale.scale(transform.scale)
			};
		}
		inline Transform inverse() const {
			Vec3F si = scale.inverse();
			return { si, position.scale(si) * -1 };
		}
		inline Transform operator/(Transform const &transform) const {
			return operator*(transform.inverse());
		}
	};
}