#pragma once

#include "animation.h"
#include <cctype>
#include <vector>
#include <map>
#include <string>

using namespace std;

static constexpr int const fs = 8;

class Text : public Paintable {
	vector<pair<Texture *, int>> sprites;
public:
	static map<char, Layer *> textures;
	Text(char const *content, Layer &foreground) {
		char c;
		for(int i = 0, x = 0; c = content[i]; ++i, x += fs) {
			if(!isalnum(c))
				continue;
			if(isupper(c))
				c += 'a' - 'A';
			Layer *fg_copy = new Layer(foreground);
			Texture *sprite = new Texture(
				{ fs, fs }, { 0, 0 },
				fg_copy, Text::textures[c]
			);
			sprites.push_back(pair(sprite, x));
		}
	}
	~Text() {
		for(auto p : sprites) {
			Texture *s = p.first;
			delete s->foreground;
			delete s;
		}
	}
	void paintOn(HDC &hdc, Int2 position) const {
		for(auto p : sprites) {
			auto sprite = p.first;
			auto x = p.second;
			Int2 current = position;
			current.x += x;
			sprite->paintOn(hdc, current);
		}
	}
};