#pragma once

#include <initializer_list>
#include <utility>

namespace Win32GameEngine {
	using namespace std;

	template<unsigned D, typename T, typename Impl = void>
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
		Impl operator*(S s) const {
			Impl res;
			for(unsigned i = 0; i < D; ++i)
				res[i] = (T)(at(i) * s);
			return res;
		}
		template<typename V>
		Impl operator+(V const &v) const {
			Impl res;
			for(unsigned i = 0; i < D; ++i)
				res[i] = at(i) + v.at(i);
			return res;
		}
		template<typename V>
		Impl operator-(V const &v) const {
			return operator+(v * -1);
		}
		template<typename V>
		T dot(V const &v) {
			T res = 0;
			for(unsigned i = 0; i < D; ++i)
				res += at(i) * v.at(i);
			return res;
		}
		template<typename V>
		Impl scale(V const &v) const {
			Impl res;
			for(unsigned i = 0; i < D; ++i)
				res[i] = at(i) * v.at(i);
			return res;
		}
		Impl inverse() const {
			Impl res;
			for(unsigned i = 0; i < D; ++i)
				res[i] = (T)1 / at(i);
			return res;
		}
	};

	template<unsigned D, typename T, unsigned step, typename Impl = void>
	struct VectorAccessor : _Vector<D, T, Impl> {
		T *const data;
		VectorAccessor(T *data) : data(data) {}
		~VectorAccessor() {
			int a = 1;
		}
		virtual inline T at(unsigned i) const override { return data[i * step]; }
		virtual inline T &operator[](unsigned i) override { return data[i * step]; }
		virtual inline T const &operator[](unsigned i) const override { return data[i * step]; }
	};

	template<unsigned D, typename T>
	struct Vector : VectorAccessor<D, T, 1, Vector<D, T>> {
		using Base = VectorAccessor<D, T, 1, Vector<D, T>>;
		Vector() : Base(new T[D]) {
			for(unsigned i = 0; i < D; ++i)
				this->operator[](i) = 0;
		}
		// Copy constructor
		Vector(Vector<D, T> const &v) : Vector() {
			operator=<T>(v);
		}
		Vector(initializer_list<T> list) : Vector() {
			T const *arr = list.begin();
			for(unsigned i = 0, m = min(list.size(), D); i < m; ++i)
				this->operator[](i) = arr[i];
		}
		~Vector() { delete[] this->data; }
		template<typename U>
		Vector<D, T> &operator=(Vector<D, U> const &v) {
			for(unsigned i = 0; i < D; ++i)
				this->operator[](i) = (T)v.at(i);
			return *this;
		}
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
		using Row = VectorAccessor<ID, T, 1, In>;
		using Col = VectorAccessor<OD, T, ID, Out>;
		static constexpr unsigned size = ID * OD;
		T data[size];
		Matrix() {
			for(unsigned i = 0; i < size; ++i)
				data[i] = 0;
		}
		inline Row row(unsigned i) { return Row((T *)data + i * ID); }
		inline Col col(unsigned i) { return Col((T *)data + i); }
		virtual Out operator()(In vector) override {
			Out res;
			for(unsigned i = 0; i < OD; ++i)
				res[i] = row(i).dot(vector);
			return res;
		}
	};

	struct Transform : Matrix<4U, 4U, float> {
		//
	};
}