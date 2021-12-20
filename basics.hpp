#pragma once

#define NOMINMAX

#include <windows.h>

#include "linear.hpp"

namespace Win32GameEngine {
	using namespace std;

	using String = LPTSTR;
	using ConstString = LPCTSTR;

	template <template <typename...> typename Template, typename... Args>
	void _derived_from_template(const Template<Args...> &);

	template <typename T, template <typename...> typename Template>
	concept derived_from_template = requires(const T & t) {
		_derived_from_template<Template>(t);
	};
}