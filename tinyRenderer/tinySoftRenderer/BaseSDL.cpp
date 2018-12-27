#include "BaseSDL.h"

//public和private内声明的变量，初始化顺序
//在VisualC++中，我测试的是按照变量声明顺序（不看public和private）
BaseSDL::BaseSDL(int w, int h, const char * windowName):screenWidth(w),screenHeight(h)
{
	checkValid(SDL_Init(SDL_INIT_VIDEO), "Unable to init SDL");

	mainwindow = SDL_CreateWindow(windowName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
	checkValid(mainwindow, "Unable to create window");

	render = SDL_CreateRenderer(mainwindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	checkValid(render, "Unable to create renderer");

	mainRt = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
}

BaseSDL::~BaseSDL()
{
	cleanup(mainRt);
	cleanup(render);
	cleanup(mainwindow);
	SDL_Quit();
}

void BaseSDL::Render()
{
	SDL_RenderPresent(render);
}

void BaseSDL::Clean(RenderContext *renderContext, Color color)
{
	int col = (color.GetA() << 24) | (color.GetR() << 16) | (color.GetG() << 8) | (color.GetB() << 0);
	//memset(void *s, int ch,size_t n);中ch实际范围应该在0~~255，因为该函数只能取ch的后八位赋值给你所输入的范围的每个字节
	//需要验证
	//其实就是按字节set的
	memset(renderContext->backBuffer, col, renderContext->width*renderContext->height*renderContext->bpp);
	std::fill(renderContext->depthBuffer, renderContext->depthBuffer + renderContext->width*renderContext->height, -FLT_MAX);
}

void BaseSDL::SwapBuffer(RenderContext *renderContext)
{
	void* pixel;
	int pitch;

	SDL_LockTexture(mainRt, NULL, &pixel, &pitch);
	memcpy(pixel, renderContext->backBuffer, renderContext->width*renderContext->height*renderContext->bpp);
	SDL_UnlockTexture(mainRt);

	SDL_RenderCopy(render, mainRt, NULL, NULL);
	SDL_RenderPresent(render);
}

void BaseSDL::HandleKeyEvent(SDL_Keysym * keysym)
{
	switch (keysym->sym)
	{
	case SDLK_ESCAPE:
		this->~BaseSDL();
		break;
	case SDLK_SPACE:
		break;

	default:
		break;
	}
}

void BaseSDL::HandleEvents()
{
	// Our SDL event placeholder.  
	SDL_Event event;
	//Grab all the events off the queue.  
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			// Handle key Event  
			HandleKeyEvent(&event.key.keysym);
			break;
		case SDL_QUIT:
			// Handle quit requests (like Ctrl-c).  
			SDL_Quit();
			/* Exit program. */
			exit(0);
			break;
		}
	}
}
