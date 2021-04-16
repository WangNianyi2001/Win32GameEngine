#include "scene.h"

using namespace Win32GameEngine;

map<string, Scene *> *Scene::all = nullptr;
Scene *Scene::active = nullptr;

Scene::Scene(string name, map<UINT, set<EventHandler::Handler>> handlers) :
	event_handler(EventHandler(handlers))
{
	if(!all)
		all = new map<string, Scene *>{};
	(*all)[name] = this;
}

bool Scene::switchTo(char const *name, HWND hWnd) {
	if(!Scene::all->count(name))
		return false;
	if(Scene::active)
		Scene::active->event_handler(hWnd, WM_DESTROY, NULL, NULL);
	Scene *scene = (*Scene::all)[name];
	Scene::active = scene;
	scene->event_handler(hWnd, WM_CREATE, NULL, NULL);
	return true;
}

LRESULT Scene::passEvent(HWND hWnd, UINT type, WPARAM wParam, LPARAM lParam) {
	if(Scene::active)
		return Scene::active->event_handler(hWnd, type, wParam, lParam);
	return DefWindowProc(hWnd, type, wParam, lParam);
}
