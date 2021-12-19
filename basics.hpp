#pragma once

#define NOMINMAX

#include <windows.h>
#include <initializer_list>
#include <utility>

namespace Win32GameEngine {
	using namespace std;

	using String = LPTSTR;
	using ConstString = LPCTSTR;

	template <template <typename...> typename Template, typename... Args>
	void _derived_from_template(const Template<Args...> &);

	template <typename T, template <typename...> typename Template>
	concept derived_from_template = requires(const T & t) {
		_derived_from_template<Template>(t);
	};

	template<unsigned D, typename T, typename Implem = void>
	struct _Vector {
		static constexpr unsigned dimension = D;
		virtual T at(unsigned i) const = 0;
		virtual T &operator[](unsigned i) = 0;
		virtual T const &operator[](unsigned i) const = 0;
		bool operator==(_Vector<D, T> const &v) const {
			for(unsigned i = 0; i < D; ++i) {
				if(at(i) != v.at(i))
					return false;
			}
			return true;
		}
		bool operator!=(_Vector<D, T> const &v) const {
			return !operator==(v);
		}
		template<typename S>
		Implem operator*(S s) const {
			Implem res;
			for(unsigned i = 0; i < D; ++i)
				res[i] = (T)(at(i) * s);
			return res;
		}
		template<typename V>
		Implem operator+(V const &v) const {
			Implem res;
			for(unsigned i = 0; i < D; ++i)
				res[i] = at(i) + v.at(i);
			return res;
		}
		template<typename V>
		Implem operator-(V const &v) const {
			return operator+(v * -1);
		}
		template<typename V>
		Implem scale(V const &v) const {
			Implem res;
			for(unsigned i = 0; i < D; ++i)
				res[i] = at(i) * v.at(i);
			return res;
		}
		Implem inverse() const {
			Implem res;
			for(unsigned i = 0; i < D; ++i)
				res[i] = (T)1 / at(i);
			return res;
		}
	};

	template<unsigned D, typename T>
	struct Vector : _Vector<D, T, Vector<D, T>> {
		T data[D];
		Vector() {}
		template<typename V>
		Vector(V const &v) {
			for(unsigned i = 0; i < D; ++i)
				operator[](i) = (T)v.at(i);
		}
		Vector(initializer_list<T> list) {
			T const *arr = list.begin();
			for(unsigned i = 0, m = min(list.size(), D); i < m; ++i)
				operator[](i) = arr[i];
		}
		virtual inline T at(unsigned i) const override { return data[i]; }
		virtual inline T &operator[](unsigned i) override { return data[i]; }
		virtual inline T const &operator[](unsigned i) const override { return data[i]; }
	};

	using Vec2I = Vector<2U, int>;
	using Vec2U = Vector<2U, unsigned>;
	using Vec2F = Vector<2U, float>;
	using Vec3F = Vector<3U, float>;
	using Vec4F = Vector<4U, float>;

	template<typename In, typename Out>
	struct LinearTransform {
		virtual Out operator()(In) = 0;
	};

	template<unsigned ID, unsigned OD, typename T>
	struct Matrix : LinearTransform<Vector<ID, T>, Vector<OD, T>> {
		using In = Vector<ID, T>;
		using Out = Vector<OD, T>;
		static constexpr unsigned size = ID * OD;
		T data[size];
		virtual Out operator()(In vector) override {
			return Out();
		}
	};

	struct Transform : Matrix<4U, 4U, float> {
		//
	};
}