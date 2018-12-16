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
#include <iostream>
#include "geometry.h"
#include "tgaimage.h"

//将第一节教程中划线部分  整合为一个头文件

//main test
//line4(10, 20, 70, 50, image, white);
//line2(70, 50, 10, 20, image, red);
//line2(10, 20, 20, 80, image, red);
//line(10, 20, 20, 80, image, red);
//image.flip_vertically();
//image.write_tga_file("line10.tga");

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	for (float t = 0; t < 1.0f; t += 0.01f) {
		int x = x0 * (1.0f - t) + x1 * t;
		int y = y0 * (1.0f - t) + y1 * t;
		image.set(x, y, color);
	}
}

//注意这个版本的问题，它默认x0比x1小
//注意当线的宽度小于高度时，画的只是点（看起来）
void line2(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	for (int x = x0; x <= x1; ++x) {
		float t = (x - x0) / (float)(x1 - x0);
		int y = y0 * (1.0f - t) + y1 * t;
		image.set(x, y, color);
	}
}

//修补line2的空洞问题
//和第一个版本 看起来差不多
//但是  效率上高点
//抓住 画的像素点都是整型的，没必要像第一个版本那样重复计算
void line3(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	bool steep = false;

	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	for (int x = x0; x <= x1; ++x) {
		float t = (x - x0) / (float)(x1 - x0);
		int y = y0 * (1.0f - t) + y1 * t;
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
	}

}

//是否能避免多次除法？  把TGAColor改为引用类型
void line4(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color) {
	bool steep = false;

	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	float derror = std::abs(dy / static_cast<float>(dx));
	float error = 0;
	int y = y0;
	for (int x = x0; x <= x1; ++x) {
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
		error += derror;
		if (error > 0.5f) {//为啥和0.5f比较  和 1.0f没感觉差别  可能太小了
			y += (y1 > y0 ? 1 : -1);
			error -= 1;
		}
	}
}

void line5(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color) {
	bool steep = false;

	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	float derror = std::abs(dy);//*2  没感觉到差别
	float error = 0;
	int y = y0;
	for (int x = x0; x <= x1; ++x) {
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
		error += derror;
		if (error > dx) {
			y += (y1 > y0 ? 1 : -1);
			error -= dx;//*2
		}
	}
}

void line5(const Vec2i& t0, const Vec2i& t1, TGAImage& image, const TGAColor& color) {
	bool steep = false;
	int x0 = t0.x;//其实这4个变量可以省略
	int y0 = t0.y;
	int x1 = t1.x;
	int y1 = t1.y;

	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	float derror = std::abs(dy);//*2  没感觉到差别
	float error = 0;
	int y = y0;
	for (int x = x0; x <= x1; ++x) {
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
		error += derror;
		if (error > dx) {
			y += (y1 > y0 ? 1 : -1);
			error -= dx;//*2
		}
	}
}
