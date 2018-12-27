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
#include <iostream>

template <typename T, typename ... Args>
void cleanup(T* t, Args... args) {
	cleanup(t);
	cleanup(args...);
}

//就SDL来说有必要吗？
//当然，如果其他对象支持移动操作，更推荐
/*
template <typename T, typename ... Args>
void cleanup(T* t, Args&&... args) {
	cleanup(t);
	cleanup(std::forward<Args>(args)...);
}*/

template<>
inline void cleanup<SDL_Window>(SDL_Window* window) {
	SDL_DestroyWindow(window);
}

template<>
inline void cleanup<SDL_Renderer>(SDL_Renderer* ren) {
	SDL_DestroyRenderer(ren);
}

template<>
inline void cleanup<SDL_Texture>(SDL_Texture* tex) {
	SDL_DestroyTexture(tex);
}