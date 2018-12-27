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
#include <fstream>
#include <string>
#include "Color.h"
#include "BaseSDL.h"
#include "Rasterizer.h"
#include "model.h"

//程序简单，尽量避免全局变量
int width = 800;
int height = 600;

void test1() {
	void* pixel = new char[12];
	int* p = new int[3];
	p[0] = 1; p[1] = 2; p[2] = 3;
	memcpy(pixel, p, sizeof(int) * 3);

	auto data = static_cast<int*>(pixel);

	std::cout << data[0] << data[1] << data[2] << "\n";
}

void test2() {
	void* pixel = new char[12];
	memset(pixel, 1, 12);
	auto data = static_cast<int*>(pixel);

	std::cout << data[0] << data[1] << data[2] << "\n";
}

inline Vec3f Point() {
	return Vec3f(width * Random::Value(), height * Random::Value(), 0);
}

int main(int argc, char* args[]){
	//test1();

	RenderContext renderContext;
	renderContext.width = width;
	renderContext.height = height;
	renderContext.bpp = sizeof(int);
	renderContext.backBuffer = new int[renderContext.width * renderContext.height];
	renderContext.depthBuffer = new float[renderContext.width * renderContext.height];

	Rasterizer rasterizer(&renderContext);
	Color c(255, 255, 255);
	BaseSDL* sdl = new BaseSDL(renderContext.width, renderContext.height, "hello");

	Model* model = new Model("../tinyRenderer/obj/african_head/african_head.obj");

	while (true) {
		sdl->HandleEvents();
		sdl->Clean(&renderContext, Color(0, 0, 0, 255));
		/*
		//Part 1: test pixel
		for (int i = 0; i < 100; i++)
		{//问题出现在Random上
			rasterizer.DrawPixel(width * Random::Value(), height * Random::Value(), c);
		}*/

		//Part 2: draw lines
		/*
		for (int i = 0; i < 10; i++)
		{
			rasterizer.DrawOneline(Vec2i(width * Random::Value(), height * Random::Value()), Vec2i(width * Random::Value(), height * Random::Value()), c);
		}*/

		//Part 3: draw triangles
		/*
		for (int i = 0; i < 10; i++)
		{
			rasterizer.triangle(Point(), Point(), Point(), Color::RandomColor());
		}*/

		for (int i = 0; i < model->nfaces(); ++i) {
			Vec3f screen_coord[3];
			Vec3f tmp[3];
			Vec2f UV[3];
			for (int j = 0; j < 3; ++j) {
				//注意model->vert(i, j);  
				//model中的faces_是三维数组，[][][]从左到右依次代表第几个face，face中分3个x/x/x 在访问第几个x
				//f x/x/x x/x/x x/x/x
				//看其实现会发现  i代表faceVer[i]
				//相当于
				//auto faceVer = model->face(i, 0);
				//tmp[i] = model->vert(faceVer[i]);
				tmp[j] = model->vert(i, j);
				UV[j] = model->uv(i, j);
				screen_coord[j] = Vec3f(int((tmp[j].x + 1.0f)*width / 2.0f), int((tmp[j].y + 1.0f)*height / 2.0f), tmp[0].z);
			}
			rasterizer.triangleUV(screen_coord[0], screen_coord[1], screen_coord[2], UV, model);
		}
		rasterizer.flip_vertically();
		sdl->SwapBuffer(&renderContext);
	}

	delete sdl;

	return 0;
}