#pragma once

#define NOMINMAX

#include <windows.h>
#include <filesystem>

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
	struct File {
		unsigned char *data;
		int size;
		File(ConstString url) : data(nullptr), size(-1) {
			auto path = filesystem::current_path();
			path.append(url);
			url = path.c_str();
			FILE *file = nullptr;
			_wfopen_s(&file, url, L"rb");
			if(!file)
				throw L"File not found.";
			fseek(file, 0, SEEK_END);
			size = ftell(file);
			fseek(file, 0, SEEK_SET);
			data = new unsigned char[size];
			fread(data, 1, size, file);
			fclose(file);
		}
		~File() {
			delete[] data;
		}
	};
}

#include "linear.hpp"
#include "buffer.hpp"
#include "event.hpp"
#include "window.hpp"