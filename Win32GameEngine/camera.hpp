#pragma once

#include "buffer.hpp"
#include "gameobject.hpp"

namespace Win32GameEngine {

	template<typename Direction>
	class Camera : public Component {
		Bitmap bitmap;
	protected:
		virtual Color sample(Direction direction) {
			default_random_engine e();
			uniform_int_distribution<Color> u;
			return u(e);
		}
	public:
		virtual void onupdate() override {
			//
		}
	};
}