#pragma once

#include "basics.hpp"

namespace Win32GameEngine {
	template<typename Index, typename Location = unsigned>
	struct Locator {
		using _Index = Index;
		using _Location = Location;
		virtual Location operator()(Index index) const = 0;
	};

	template<typename Data, derived_from_template<Locator> Locator>
	struct Buffer {
		using Index = Locator::_Index;
		unsigned const size;
		Data *const data;
		Locator const locator;
		Buffer(unsigned size, Locator const locator) :
			size(size), data(new Data[size]), locator(locator) {
		}
		Buffer(Buffer<Data, Locator> const &buffer) : Buffer(buffer.size, buffer.locator) {
			memcpy(data, buffer->data, size * sizeof(Data));
		}
		virtual bool valid(Index const index) = 0;
		virtual Data *at(Index index) {
			return valid(index) ? data + locator(index) : nullptr;
		}
		virtual Data &operator[](Index index) {
			return *at(index);
		}
	};

	struct Bitmap;

	struct BitmapLocator : Locator<Vec2I> {
		Bitmap *const bitmap;
		BitmapLocator(Bitmap *const bitmap) : bitmap(bitmap) {}
		virtual unsigned operator()(Vec2I index) const override {
			return 0;
		}
	};

	using Color = unsigned __int32;
	struct Bitmap : Buffer<Color, BitmapLocator> {
		Vec2U const dimension;
		Bitmap(Vec2U dimension) :
			dimension(dimension),
			Buffer<Color, BitmapLocator>(dimension[0] * dimension[1], BitmapLocator(this)) {
		}
		virtual bool valid(Vec2I const index) override {
			int x = index.at(0), y = index.at(1);
			return x >= 0 && y >= 0 && (unsigned)x < dimension.at(0) && (unsigned)y < dimension.at(1);
		}
	};
}