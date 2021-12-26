#pragma once

#include "utils.hpp"

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
		inline Vec4F unify() const {
			return Vec4F(Vector<4, Channel>{r, g, b, a}) * (1.f / 256);
		}
		Color operator+(Color const &c) {
			if(c.a == 255)
				return c;
			Vec4F sf = unify(), cf = c.unify();
			float _a = 1 - (1 - cf[3]) * (1 - sf[3]);
			Vec3F rgb = sf * ((1 - cf[3]) * sf[3]) + cf * cf[3];
			rgb = rgb * (1 / _a);
			Vector<3, Channel> res = rgb * 256;
			return Color(res[0], res[1], res[2], Channel(_a * 256));
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
			File file(url);
			tagBITMAPFILEHEADER *header = (tagBITMAPFILEHEADER *)file.data;
			tagBITMAPINFO *info = (tagBITMAPINFO *)(file.data + sizeof(tagBITMAPFILEHEADER));
			Vec2U dimension{ (unsigned)info->bmiHeader.biWidth, (unsigned)info->bmiHeader.biHeight };
			Bitmap bitmap(dimension);
			Color *data = bitmap.data.get();
			typename Color::Channel *start = file.data + header->bfOffBits;
			switch(info->bmiHeader.biBitCount) {
			case 32:
				for(unsigned i = 0, o = 0; i < bitmap.size; ++i) {
					o = i * 4;
					data[i] = Color(start[o], start[o + 1], start[o + 2], start[o + 3]);
					if(start[o + 3] != 255) {
						int a = 1;
					}
				}
				break;
			case 24:
				for(unsigned i = 0, o = 0; i < bitmap.size; ++i) {
					o = i * 3;
					data[i] = Color(start[o], start[o + 1], start[o + 2]);
				}
				break;
			default:
				throw L"Unrecognized BMP data layout.";
			}
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