/*
*
*
*@author: Aver Jing
*@description：
*@date：
*
*
*/
#pragma once
#include <SDL.h>
#include "RenderContex.h"
#include "Color.h"
#include "cleanup.h"

//对SDL进行封装

class BaseSDL {
public:
	int screenWidth;
	int screenHeight;

	BaseSDL() {}
	BaseSDL(int w, int h, const char* windowName);
	~BaseSDL();

	void Render();
	void Clean(RenderContext*, Color);
	void SwapBuffer(RenderContext*);

	void HandleKeyEvent(SDL_Keysym* keysym);
	void HandleEvents();
private:
	SDL_Window* mainwindow;
	SDL_Texture* mainRt;
	SDL_Renderer* render;
};

template <typename T>
inline void checkValid(T* t, const char* msg) {
	if (!t) {
		std::cout << msg << " &_& " << SDL_GetError() << std::endl;
		SDL_Quit();
		exit(-1);
	}	
}

inline void checkValid(int t, const char* msg) {
	if (t < 0) {
		std::cout << msg << " &_& " << SDL_GetError() << std::endl;
		SDL_Quit();
		exit(-1);
	}	
}