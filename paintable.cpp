#include "paintable.h"

void Scalable::paintOn(HDC &dest, Int2 position) const {
	this->paintOn(dest, position, { 1.0f, 1.0f });
}