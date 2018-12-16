/*
*
*
*@author: Aver Jing
*@description：
*@date：
*
*
*/

#include <iostream>
#include <string>
#include <SDL.h>

using std::string;

//https://adolfans.github.io/sdltutorialcn/blog/2013/01/28/lesson-2-dont-put-everything-in-main/

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

template<class T>
inline void error(T* t) {//不确定是否会inline   
	if (!t) {
		std::cout << SDL_GetError() << std::endl;
		throw std::runtime_error("create error");
	}
}

//先使用SDL_Surface加载图片，在将其转化为纹理SDL_Texture
SDL_Texture* LoadTexture(const string& file) {
	SDL_Surface* loadedimage = nullptr;
	SDL_Texture* tex = nullptr;

	loadedimage = SDL_LoadBMP(file.c_str());
	error(loadedimage);

	tex = SDL_CreateTextureFromSurface(renderer, loadedimage);
	SDL_FreeSurface(loadedimage);

	return tex;
}

//指定texture绘制的位置
void ApplySurface(int x, int y, SDL_Texture* tex, SDL_Renderer* ren) {
	SDL_Rect pos;
	pos.x = x;
	pos.y = y;

	SDL_QueryTexture(tex, NULL, NULL, &pos.w, &pos.h);

	SDL_RenderCopy(ren, tex, NULL, &pos);
}

int main(int argc, char* args[]){
	SDL_Init(SDL_INIT_EVERYTHING);
	//SDL_WINDOWPOS_CENTERED Used to indicate that the window position should be centered.
	window = SDL_CreateWindow("Lession2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	error(window);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	error(renderer);

	SDL_Texture* background = nullptr, *image = nullptr;
	background = LoadTexture("SDL_Lession/Lession02/background.bmp");
	image = LoadTexture("SDL_Lession/Lession02/image.bmp");

	error(background);
	error(image);

	SDL_RenderClear(renderer);

	//可以使用for
	int bW, bH;
	SDL_QueryTexture(background, NULL, NULL, &bW, &bH);
	ApplySurface(0, 0, background, renderer);
	ApplySurface(bW, 0, background, renderer);
	ApplySurface(0, bH, background, renderer);
	ApplySurface(bW, bH, background, renderer);

	int iW, iH;
	SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
	int x = SCREEN_WIDTH / 2 - iW / 2;
	int y = SCREEN_HEIGHT / 2 - iH / 2;
	ApplySurface(x, y, image, renderer);

	SDL_RenderPresent(renderer);
	SDL_Delay(5000);

	SDL_DestroyTexture(background);
	SDL_DestroyTexture(image);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}	