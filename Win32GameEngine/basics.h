#pragma once

#include <windows.h>

namespace Win32GameEngine {
	using String = LPTSTR;
	using ConstString = LPCTSTR;

	template <template <class...> class Template, class... Args>
	void _derived_from_template(const Template<Args...> &);

	template <class T, template <class...> class Template>
	concept derived_from_template = requires(const T & t) {
		_derived_from_template<Template>(t);
	};
}