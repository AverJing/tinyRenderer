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
#include <vector>
#include "geometry.h"
#include "tgaimage.h"
#include "model.h"

TGAColor white(255, 255, 255);
TGAColor red(255, 0, 0);

//line.h
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

//修正 当高度大于宽度时，出现空洞
void line3(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		//当高度大于宽度，高度和宽度互换
		//当在设置渲染结果时，在反过来
		steep = true;
		std::swap(x0, y0);
		std::swap(x1, y1);
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	float temp = 1.0f / (x1 - x0);
	for (int x = x0; x <= x1; ++x) {
		float t = temp * (x - x0);
		int y = y0 * (1 - t) + y1 * t;
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
	}
}

//性能优化
void line4(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		//当高度大于宽度，高度和宽度互换
		//当在设置渲染结果时，在反过来
		steep = true;
		std::swap(x0, y0);
		std::swap(x1, y1);
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	float derror = std::abs(float(y1 - y0) / (x1 - x0));
	int y = y0;
	float error = 0.0f;
	for (int x = x0; x <= x1; ++x) {	
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
		error += derror;
		if (error > 0.5f) {
			y += (y1 > y0) ? 1 : -1;
			error -= 1;
		}
	}
}

//进一步优化
void line5(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		//当高度大于宽度，高度和宽度互换
		//当在设置渲染结果时，在反过来
		steep = true;
		std::swap(x0, y0);
		std::swap(x1, y1);
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = std::abs(y1 - y0);
	int y = y0;
	int error = 0;
	for (int x = x0; x <= x1; ++x) {
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
		error += dy;
		if (error >= dx) {
			y += (y1 > y0) ? 1 : -1;
			error -= dx;
		}
	}
}

const int width = 800;
const int height = 800;

int main(){
	/*
	TGAImage image(300, 300, TGAImage::RGB);
	line5(13, 20, 80, 40, image, white);
	line5(20, 13, 40, 80, image, red);
	line5(80, 40, 13, 20, image, red);

	image.flip_vertically();
	image.write_tga_file("line/line5.tga");*/

	TGAImage image(width, height, TGAImage::RGB);
	auto model = new Model("obj/african_head/african_head.obj");

	for (int i = 0; i < model->nfaces(); ++i) {
		std::vector<int> face = model->face(i, 0);
		for (int j = 0; j < 3; ++j) {
			auto v0 = model->vert(face[j]);
			auto v1 = model->vert(face[(j + 1) % 3]);
			int x0 = (v0.x + 1.0f)*width / 2.0f;
			int y0 = (v0.y + 1.0f)*height / 2.0f;
			int x1 = (v1.x + 1.0f)*width / 2.0f;
			int y1 = (v1.y + 1.0f)*height / 2.0f;
			//注意顶点坐标范围时-1到1
			line5(x0, y0, x1, y1, image, white);
		}
	}

	image.flip_vertically();
	image.write_tga_file("line/african.tga");
}	