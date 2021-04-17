#include "scene.h"

using namespace Win32GameEngine;

map<char const *, Scene *> *Scene::all = nullptr;
Scene *Scene::active = nullptr;

Win32GameEngine::Scene::Scene(
	char const *name, void(*init)(HWND, void *),
	map<UINT, set<EventHandler::Handler>> handlers
) :
	init(init),
	event_handler(EventHandler(handlers))
{
	if(!all)
		all = new map<char const *, Scene *>{};
	(*all)[name] = this;
}

bool Scene::switchTo(char const *name, HWND hWnd, void *arg) {
	if(!Scene::all->count(name))
		return false;
	Scene *scene = (*Scene::all)[name];
	Scene::active = scene;
	(*scene->init)(hWnd, arg);
	return true;
}

LRESULT Scene::passEvent(HWND hWnd, UINT type, WPARAM wParam, LPARAM lParam) {
	if(Scene::active)
		return Scene::active->event_handler(hWnd, type, wParam, lParam);
	return DefWindowProc(hWnd, type, wParam, lParam);
}