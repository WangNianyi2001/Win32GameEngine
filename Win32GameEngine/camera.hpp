#pragma once

#include "buffer.hpp"
#include "gameobject.hpp"

namespace Win32GameEngine {

	class Camera : public Component {
		using Index = Bitmap::_Index;
		Bitmap bitmap;
	protected:
		virtual Color sample(Index direction) {
			using Channel = Color::Channel;
			return Color(
				rand(),
				rand(),
				rand(),
				direction.at(0) > bitmap.dimension.at(0) / 2 ? 0xff : 0x88
			).premul();
		}
	public:
		Camera(Vec2I dim) : bitmap(dim) {
			//
		}
		virtual void onupdate() override {
			resample();
		}
		// Temporary painting function
		void painton(HDC hdc) {
			HDC _hdc = CreateCompatibleDC(hdc);
			HBITMAP hb = bitmap.gethandle();
			SelectObject(_hdc, hb);
			int w = bitmap.dimension.at(0), h = bitmap.dimension.at(1);
			AlphaBlend(
				hdc, 0, 0, w, h,
				_hdc, 0, 0, w, h,
				BLENDFUNCTION{ AC_SRC_OVER, 0, 255, AC_SRC_ALPHA }
			);
			DeleteObject(_hdc);
		}
		void resample() {
			for(Index pos{ 0, 0 }; ; ++pos[0]) {
				if(pos.at(0) == bitmap.dimension.at(0)) {
					pos[0] = 0;
					++pos[1];
					if(pos.at(1) == bitmap.dimension.at(1))
						break;
				}
				bitmap[pos] = sample(pos);
			}
		}
	};
}