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
		shared_ptr<Data> const data;
		Buffer(unsigned size, Data *const data) : size(size), data(data) {}
		Buffer(unsigned size, shared_ptr<Data> data) : size(size), data(data) {}
		Buffer(unsigned size) : Buffer(size, new Data[size]) {}
		Buffer(Buffer<Data, Index> const &buffer) : Buffer(buffer.size, buffer.data) {}
		virtual unsigned locate(Index index) const = 0;
		virtual bool valid(Index const index) const = 0;
		template<typename Index>
		inline Data *at(Index index) const {
			return valid(index) ? data.get() + locate(index) : nullptr;
		}
		virtual inline Data &operator[](Index index) const {
			return *at(index);
		}
		void clear() const {
			memset(data.get(), 0, sizeof(Data) * size);
		}
	};

	struct Color {
		using Channel = unsigned __int8;
		Channel b, g, r, a;
		Color(Channel r, Channel g, Channel b, Channel a) : r(r), g(g), b(b), a(a) {}
		Color() : Color(0, 0, 0, 0) {}
		Color(Channel r, Channel g, Channel b) : Color(r, g, b, 255U) {}
		Color operator+(Color const &c) {
			return c;	// TODO
		}
	};

	struct Bitmap : Buffer<Color, Vec2I> {
		Vec2U const dimension;
		HBITMAP handle = nullptr;
		Bitmap(Vec2U dimension, shared_ptr<Color> data) :
			Buffer<Color, Vec2I>(dimension[0] * dimension[1], data), dimension(dimension) {}
		Bitmap(Vec2U dimension) : Bitmap(dimension, shared_ptr<Color>(new Color[dimension[0] * dimension[1]])) {}
		Bitmap(Bitmap const &bitmap) : Bitmap(bitmap.dimension, bitmap.data) {}
		~Bitmap() {
			handle && DeleteObject(handle);
		}
		static Bitmap fromfile(ConstString url) {
			HBITMAP handle = (HBITMAP)LoadImage(NULL, url, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			if(!handle) {
				if(!GetLastError())
					throw L"Invalid BMP bit format.";
				throw L"Failed to load bitmap file.";
			}
			BITMAP bm;
			GetObject(handle, sizeof(BITMAP), &bm);
			Bitmap bitmap({ (unsigned)bm.bmWidth, (unsigned)bm.bmHeight });
			GetBitmapBits(handle, bitmap.size * sizeof(Color), bitmap.data.get());
			DeleteObject(handle);
			return bitmap;
		}
		HBITMAP gethandle() {
			handle && DeleteObject(handle);
			handle = CreateBitmap(dimension[0], dimension[1], 1U, 32U, data.get());
			return handle;
		}
		virtual unsigned locate(Vec2I index) const override {
			return index.at(1) * dimension.at(0) + index.at(0);
		}
		virtual bool valid(Vec2I const index) const override {
			int x = index.at(0), y = index.at(1);
			return x >= 0 && y >= 0 && (unsigned)x < dimension.at(0) && (unsigned)y < dimension.at(1);
		}
	};
}