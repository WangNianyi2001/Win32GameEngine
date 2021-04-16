#pragma once

#include <windows.h>

template<typename T> struct Pair { T x, y; };

using Int2 = Pair<int>;
using Float2 = Pair<float>;

class Paintable {
public:
	virtual void paintOn(HDC &dest, Int2 position) const = 0;
};

class Scalable : public Paintable {
public:
	virtual void paintOn(HDC &dest, Int2 position, Float2 scale = { 1.0f, 1.0f }) const = 0;
	virtual void paintOn(HDC &dest, Int2 position) const;
};