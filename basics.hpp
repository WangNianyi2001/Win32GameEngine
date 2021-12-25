#pragma once

#define NOMINMAX

#include <windows.h>

#include "linear.hpp"

namespace Win32GameEngine {
	using namespace std;

	using String = LPTSTR;
	using ConstString = LPCTSTR;

	template<template <typename...> typename Template, typename... Args>
	void _derived_from_template(const Template<Args...> &);

	template<typename T, template <typename...> typename Template>
	concept derived_from_template = requires(const T & t) {
		_derived_from_template<Template>(t);
	};

	struct Ticker {
		ULONGLONG const start;
		ULONGLONG last, rate;
		Ticker(ULONGLONG rate) : start(GetTickCount64()), last(start), rate(rate) {}
		Ticker() : Ticker(0) {}
		static inline ULONGLONG get() { return GetTickCount64(); }
		inline ULONGLONG since() const { return get() - start; }
		inline ULONGLONG elapsed() const { return get() - last; }
		inline void tick() { last = get(); }
		void setrate(ULONGLONG rate) { this->rate = rate; }
		inline bool isup() const { return rate && elapsed() >= rate; }
	};
}