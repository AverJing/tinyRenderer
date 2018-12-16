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
#include <SDL.h>

//https://adolfans.github.io/sdltutorialcn/blog/2013/01/26/lesson-1-hello-world/

template<class T>
void error(T* t) {//不确定是否会inline   
	if (!t) {
		std::cout << SDL_GetError() << std::endl;
		throw std::runtime_error("create error");
	}
}

int main(int argc, char* args[]){

	SDL_Window* screen = nullptr;

	//启动SDL
	SDL_Init(SDL_INIT_EVERYTHING);//返回-1表示失败

	//设置窗口
	screen = SDL_CreateWindow("hello", 100, 100, 800, 800, SDL_WINDOW_SHOWN);
	//SDL_CreateWindow参数含义  标题，窗口所打开位置的x和y坐标，宽度w和高度h。 最后一个是窗口的各种flag
	error(screen);

	SDL_Renderer* ren = nullptr;
	ren = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	//函数注释解释的很清楚。  -1代表让SDL自动选择适合我们指定选项的驱动
	//SDL_RENDERER_ACCELERATED  如其名 硬件加速
	//SDL_RENDERER_PRESENTVSYNC  以显示器刷新率来更新画面
	error(ren);

	//加载图像
	SDL_Surface* bmp = nullptr;
	bmp = SDL_LoadBMP("SDL_Lession/Lession01/hello2.bmp");
	error(bmp);

	//为了使用硬件加速绘制，我们必须把SDL_Surface转化为SDL_Texture，这样renderer才能绘制
	SDL_Texture* tex = nullptr;
	tex = SDL_CreateTextureFromSurface(ren, bmp);
	error(tex);
	SDL_FreeSurface(bmp);//释放资源，必须借助它

	SDL_RenderClear(ren);
	SDL_RenderCopy(ren, tex, NULL, NULL);
	//注意这两个NULL，第一个NULL是指向源矩形的指针，也就是从图像上剪裁下来一块矩形
	//第二个NULL是指向目标矩形的指针。
	SDL_RenderPresent(ren);

	SDL_Delay(2000);//milliseconds

	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(screen);

	SDL_Quit();

	return 0;
}	