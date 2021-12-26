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
		inline bool operator!=(_Vector<D, T> const &v) const {
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
		T dot(V const &v) const {
			T res = 0;
			for(unsigned i = 0; i < D; ++i)
				res += at(i) * v.at(i);
			return res;
		}
		inline T module() const { return (T)sqrt(dot(*this)); }
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

	template<unsigned D, typename T>
	struct Vector : _Vector<D, T, Vector<D, T>> {
		T data[D];
		Vector() {
			for(unsigned i = 0; i < D; ++i)
				this->operator[](i) = 0;
		}
		// Copy constructor
		Vector(Vector<D, T> const &v) : Vector() {
			this->operator=(v);
		}
		Vector(initializer_list<T> list) : Vector() {
			T const *arr = list.begin();
			for(unsigned i = 0, m = min(list.size(), D); i < m; ++i)
				this->operator[](i) = arr[i];
		}
		Vector(T *array) {
			for(unsigned i = 0; i < D; ++i)
				this->operator[](i) = array[i];
		}
		template<typename V>
		Vector(V const &ref) { operator=(ref); }
		template<typename V>
		Vector<D, T> &operator=(V const &v) {
			for(unsigned i = 0; i < D; ++i)
				operator[](i) = (T)v.at(i);
			return *this;
		}
		virtual inline T at(unsigned i) const override { return data[i]; }
		virtual inline T &operator[](unsigned i) override { return data[i]; }
		virtual inline T const &operator[](unsigned i) const override { return data[i]; }
	};

	template<unsigned D, typename T, int step, typename Impl = void>
	struct VectorAccessor : _Vector<D, T, Impl> {
		T *const data;
		VectorAccessor() : data(nullptr) {}
		VectorAccessor(T *data) : data(data) {}
		VectorAccessor(VectorAccessor<D, T, step, Impl> const &vector) : VectorAccessor(vector.data) {}
		template<typename V>
		VectorAccessor(V const &ref) : VectorAccessor(ref.data) {}
		template<typename V>
		VectorAccessor<D, T, step, Impl> &operator=(V const &v) {
			constexpr unsigned m = min(v.dimension, D);
			for(unsigned i = 0; i < m; ++i)
				operator[](i) = (T)v.at(i);
			return *this;
		}
		virtual inline T at(unsigned i) const override { return data[i * step]; }
		virtual inline T &operator[](unsigned i) override { return data[i * step]; }
		virtual inline T const &operator[](unsigned i) const override { return data[i * step]; }
		Vector<D, T> deref() const {
			Vector<D, T> res;
			for(unsigned i = 0; i < D; ++i)
				res[i] = at(i);
			return res;
		}
	};

	using Vec2I = Vector<2U, int>;
	using Vec2U = Vector<2U, unsigned>;
	using Vec2F = Vector<2U, float>;
	using Vec3F = Vector<3U, float>;
	using Vec4F = Vector<4U, float>;

	template<unsigned OD, unsigned ID, typename T>
	struct Matrix {
		using In = Vector<ID, T>;
		using Out = Vector<OD, T>;
		using Row = VectorAccessor<ID, T, 1, In>;
		using Col = VectorAccessor<OD, T, ID, Out>;
		static constexpr unsigned diagonal_size = OD < ID ? OD : ID;
		using Diag = VectorAccessor<diagonal_size, T, ID + 1, Vector<diagonal_size, T>>;
		static constexpr unsigned size = ID * OD;
		T data[size];
		Matrix() {
			for(unsigned i = 0; i < size; ++i)
				data[i] = 0;
		}
		Matrix(Matrix<OD, ID, T> const &matrix) {
			for(unsigned i = 0; i < size; ++i)
				data[i] = matrix.data[i];
		}
		Matrix(initializer_list<T> list) : Matrix() {
			T const *arr = list.begin();
			for(unsigned i = 0, m = min(list.size(), size); i < m; ++i)
				data[i] = arr[i];
		}
		Matrix(initializer_list<initializer_list<T>> list) : Matrix() {
			initializer_list<T> const *rows = list.begin();
			for(unsigned i = 0, m = min(list.size(), OD); i < m; ++i) {
				T const *arr = rows[i].begin();
				Row _row = row(i);
				for(unsigned j = 0; j < min(rows[i].size(), ID); ++j)
					_row[j] = arr[j];
			}
		}
		inline Row row(unsigned i) const { return Row((T *)data + i * ID); }
		inline Col col(unsigned i) const { return Col((T *)data + i); }
		inline Diag diag() const { return Diag((T *)data); }
		virtual Out operator()(In vector) const {
			Out res;
			for(unsigned i = 0; i < OD; ++i)
				res[i] = row(i).dot(vector);
			return res;
		}
		template<typename M>
		Matrix<M::Out::dimension, ID, T> compose(M const &matrix) const {
			Matrix<M::Out::dimension, ID, T> res;
			for(unsigned c = 0; c < ID; ++c)
				res.col(c) = operator()(matrix.col(c));
			return res;
		}
	};

	template<unsigned D, typename T>
	struct SquareMatrix : Matrix<D, D, T> {
		using Base = Matrix<D, D, T>;
		SquareMatrix() : Base() {}
		SquareMatrix(Matrix<D, D, T> const &matrix) : Base(matrix) {}
		SquareMatrix(initializer_list<T> list) : Base(list) {}
		SquareMatrix(initializer_list<initializer_list<T>> list) : Base(list) {}
		SquareMatrix<D, T> inverse() const {
			using Augmented = Matrix<D, 2U * D, T>;
			// Invert a square matrix by Gaussian elimination.
			// Create the augmented matrix.
			Augmented augmented;
			for(unsigned c = 0; c ^ D; ++c) {
				augmented.col(c) = this->col(c);
				augmented.col(c + D)[c] = 1;
			}
			// Perform Gaussian elimination.
			// Eliminate the bottom-triangle part
			for(unsigned r = 0; r ^ D; ++r) {
				typename Augmented::Row row = augmented.row(r);
				// Find the first row not above the r-th row
				// with a non-zero component at r-th column.
				unsigned _r = r;	// The index of the above-said row.
				for(; _r < D && augmented.row(_r)[r] == 0; ++_r);
				// If found, swap it to r-th row.
				if(_r != r && _r != D) {
					Vector<2 * D, T> temp = row.deref();
					row = augmented.row(_r).deref();
					augmented.row(_r) = temp;
				}
				// For each row below the r-th row, eliminate
				// their component at r-th row.
				for(_r = r + 1; _r < D; ++_r) {
					typename Augmented::Row target = augmented.row(_r);
					target = target - row * (target[r] / row[r]);
				}
			}
			// Eliminate the top-triangle part.
			for(unsigned r = D - 1; ~r; --r) {
				typename Augmented::Row row = augmented.row(r);
				for(unsigned _r = r - 1; ~_r; --_r) {
					typename Augmented::Row target = augmented.row(_r);
					target = target - row * (target[r] / row[r]);
				}
			}
			// Scale by inverted diagonal.
			for(unsigned r = 0; r ^ D; ++r) {
				typename Augmented::Row row = augmented.row(r);
				row = row * (1 / row[r]);
			}
			// Transfer the right part of the augmented matrix into the result.
			SquareMatrix<D, T> res;
			for(unsigned i = 0; i < D; ++i)
				res.col(i) = augmented.col(i + D);
			return res;
		}
	};
}