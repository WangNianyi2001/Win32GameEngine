#include "scene.h"

using namespace Win32GameEngine;

Scene::Scene(char const *name) : event_handler(EventHandler()) {
	scenes.insert(pair(name, this));
}

bool switchScene(char const *name, HWND hWnd) {
	if(!scenes.count(name))
		return false;
	if(active_scene)
		active_scene->event_handler(hWnd, WM_DESTROY, NULL, NULL);
	Scene *scene = scenes[name];
	active_scene = scene;
	scene->event_handler(hWnd, WM_CREATE, NULL, NULL);
	return true;
}
