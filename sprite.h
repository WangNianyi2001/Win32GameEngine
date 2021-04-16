#pragma once

#include "paintable.h"
#include <vector>
#include <map>
#include <string>

using namespace std;

class Sprite : public Scalable {
protected:
	Sprite() = default;
public:
	static string const default_name;
	map<string, Scalable *> appearances;
	map<string, Scalable *>::iterator current;
	void paintOn(HDC &dest, Int2 position, Float2 scale = { 1.0f, 1.0f }) const;
	void switchAppearance(string name);
	Sprite(Scalable *appearance);
	Sprite(vector<pair<string, Scalable *>> appearances, string begin);
};