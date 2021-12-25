#pragma once

#include "basics.hpp"
#include <filesystem>

namespace Win32GameEngine {
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