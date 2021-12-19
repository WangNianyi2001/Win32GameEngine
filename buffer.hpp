#pragma once

#include "basics.hpp"

namespace Win32GameEngine {
	template<typename Index>
	struct Locator {
		using _Index = Index;
	};

	template<typename Data, typename Index>
	struct Buffer {
		using _Index = Index;
		unsigned const size;
		Data *const data;
		Buffer(unsigned size) : size(size), data(new Data[size]) {}
		Buffer(Buffer<Data, Index> const &buffer) : Buffer(buffer.size) {
			memcpy(data, buffer->data, size * sizeof(Data));
		}
		~Buffer() {
			delete[] data;
		}
		virtual unsigned locate(Index index) const = 0;
		virtual bool valid(Index const index) = 0;
		template<typename Index>
		inline Data *at(Index index) {
			return valid(index) ? data + locate(index) : nullptr;
		}
		virtual inline Data &operator[](Index index) {
			return *at(index);
		}
	};

	struct Color {
		using Channel = unsigned __int8;
		Channel r, g, b, Vector;
		Color() = default;
		template<typename T>
		Color(T r, T g, T b, T Vector) : r(r), g(g), b(b), Vector(Vector) {}
		Color premul() {
			return Color{
				(Channel)(r * Vector / 0xff),
				(Channel)(g * Vector / 0xff),
				(Channel)(b * Vector / 0xff),
				Vector
			};
		}
	};

	struct Bitmap : Buffer<Color, Vec2I> {
		Vec2U const dimension;
		HBITMAP handle = nullptr;
		Bitmap(Vec2U dimension) : dimension(dimension), Buffer<Color, Vec2I>(dimension[0] * dimension[1]) {}
		HBITMAP gethandle() {
			handle && DeleteObject(handle);
			return handle = CreateBitmap(dimension[0], dimension[1], 1U, 32U, data);
		}
		virtual unsigned locate(Vec2I index) const override {
			return index.at(1) * dimension.at(0) + index.at(0);
		}
		virtual bool valid(Vec2I const index) override {
			int x = index.at(0), y = index.at(1);
			return x >= 0 && y >= 0 && (unsigned)x < dimension.at(0) && (unsigned)y < dimension.at(1);
		}
		~Bitmap() {
			handle && DeleteObject(handle);
		}
	};
}