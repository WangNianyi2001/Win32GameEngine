#include "sprite.h"

string const Sprite::default_name = "default";

void Sprite::paintOn(HDC &dest, Int2 position, Float2 scale) const {
	current->second->paintOn(dest, position, scale);
}

void Sprite::switchAppearance(string name) {
	if(!appearances.count(name))
		return;
	current = appearances.find(name);
}

Sprite::Sprite(Scalable *appearance) {
	appearances.insert(pair(Sprite::default_name, appearance));
	switchAppearance(Sprite::default_name);
}

Sprite::Sprite(vector<pair<string, Scalable *>> appearances, string begin) {
	for(auto p : appearances)
		this->appearances.insert(p);
	switchAppearance(begin);
}
