#include <windows.h>
#include "texture.h"

Layer::Layer(Int2 size) :
	hdc(CreateCompatibleDC(GetDC(NULL))),
	size(size) {
}

Layer::Layer(Layer const &reference) : Layer(reference.size) {
	hbm = CreateBitmap(size.x, size.y, 1, 32, nullptr);
	SelectObject(hdc, hbm);
	BitBlt(hdc, 0, 0, size.x, size.y, reference.hdc, 0, 0, SRCCOPY);
}

void Layer::paintOn(HDC &dest, Int2 position, int mode) const {
	BitBlt(
		dest, position.x, position.y, size.x, size.y,
		hdc, 0, 0,
		mode
	);
}

void Layer::paintOn(HDC &dest, Int2 position) const {
	paintOn(dest, position, SRCCOPY);
}

void Layer::paintOn(HDC &dest, Int2 position, Int2 dest_size, int mode) const {
	StretchBlt(
		dest, position.x, position.y, dest_size.x, dest_size.y,
		hdc, 0, 0, size.x, size.y,
		mode
	);
}

PureColor::PureColor(COLORREF color, Int2 size) : Layer(size), color(color) {
	hbm = CreateBitmap(size.x, size.y, 1, 32, nullptr);
	SelectObject(hdc, hbm);
	SelectObject(hdc, GetStockObject(NULL_PEN));
	SelectObject(hdc, CreateSolidBrush(color));
	Rectangle(hdc, 0, 0, size.x + 1, size.y + 1);
}

Bitmap::Bitmap(LPCWSTR url) : Layer({ 0, 0 }) {
	hbm = (HBITMAP)LoadImage(
		NULL, url, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE
	);
	BITMAP bm;
	GetObject(hbm, sizeof(BITMAP), &bm);
	size = { bm.bmWidth, bm.bmHeight };
	SelectObject(hdc, hbm);
}

Texture::Texture(
	Int2 size, Int2 anchor,
	Layer *foreground, Layer *mask
) :
	size(size), anchor(anchor),
	foreground(foreground), mask(mask),
	visible(true) {
	if(mask) {
		invert_mask = new Layer(*mask);
		mask->paintOn(invert_mask->hdc, { 0, 0 }, { size.x, size.y }, DSTINVERT);
		mask->paintOn(foreground->hdc, { 0, 0 }, { size.x, size.y }, SRCAND);
	}
}

void Texture::paintOn(HDC &hdc, Int2 position, Float2 scale) const {
	if(!visible)
		return;
	Int2 _position{
		(int)(position.x - scale.x * anchor.x),
		(int)(position.y - scale.y * anchor.y)
	}, _size{
		(int)(scale.x * size.x),
		(int)(scale.y * size.y)
	};
	if(mask)
		invert_mask->paintOn(hdc, _position, _size, SRCAND);
	foreground->paintOn(hdc, _position, _size, mask ? SRCPAINT : SRCCOPY);
}