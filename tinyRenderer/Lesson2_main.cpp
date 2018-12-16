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
#pragma once
#include <algorithm>
#include "tgaimage.h"
#include "geometry.h"
#include "model.h"

const int width = 800;
const int height = 800;

/*
void triangle(const Vec2i& t0, const Vec2i& t1, const Vec2i& t2, TGAImage& image, const TGAColor& color) {
	line5(t0, t1, image, color);
	line5(t1, t2, image, color);
	line5(t2, t0, image, color);
}*/

//sort vertices of the triangle by their y-coordinates
/*
void triangle(Vec2i& t0, Vec2i& t1, Vec2i& t2, TGAImage& image, const TGAColor& color) {
	//t0.y<=t2.y<=t3.y
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t1.y > t2.y) std::swap(t1, t2);
	if (t0.y > t2.y) std::swap(t0, t2);

	line5(t0, t1, image, green);
	line5(t1, t2, image, green);
	line5(t2, t0, image, red);
}*/

//draw the buttom half of the triangle by cutting it horizontally by t2.y
/*
void triangle(Vec2i& t0, Vec2i& t1, Vec2i& t2, TGAImage& image, const TGAColor& color) {
	//t0.y<=t2.y<=t3.y
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t1.y > t2.y) std::swap(t1, t2);
	if (t0.y > t2.y) std::swap(t0, t2);

	float x;
	if (t0.x == t2.x)
		x = t0.x;
	else
		x = t0.x - (t1.y - t0.y) / static_cast<float>(t2.y - t1.y)*(t0.x - t2.x);

	line5(t0, t1, image, green);
	line5(t0, Vec2i(x,t1.y), image, red);
}*/

//fully draw horizonal triangle
/*
void triangle(Vec2i& t0, Vec2i& t1, Vec2i& t2, TGAImage& image, const TGAColor& color) {
	//t0.y<=t2.y<=t3.y
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t1.y > t2.y) std::swap(t1, t2);
	if (t0.y > t2.y) std::swap(t0, t2);

	int segment_height = t1.y - t0.y;
	int total_height = t2.y - t0.y;
	//buttom horizonally
	//当然可以把1/ total_height，拿出循环
	for (int y = t0.y; y <= t1.y; ++y) {
		float alpha = static_cast<float>(y - t0.y) / total_height;   //be careful with division by zero
		float beta = static_cast<float>(y - t0.y) / segment_height;
		alpha = t0.x + (t2.x - t0.x)*alpha;
		beta = t0.x + (t1.x - t0.x)*beta;
		if (alpha > beta) std::swap(alpha, beta);
		for (int x = alpha; x <= beta; ++x) {
			image.set(x, y, color);
		}
	}

	segment_height = t2.y - t1.y;
	//up horizonally
	for (int y = t1.y; y <= t2.y; ++y) {
		float alpha = static_cast<float>(y - t0.y) / total_height;   //be careful with division by zero
		float beta = static_cast<float>(y - t1.y) / segment_height;
		alpha = t0.x + (t2.x - t0.x)*alpha;
		beta = t1.x + (t2.x - t1.x)*beta;
		if (alpha > beta) std::swap(alpha, beta);
		for (int x = alpha; x <= beta; ++x) {
			image.set(x, y, color);
		}
	}
}*/

//optimization
//make it a bit less readable, but more handy for modifications/maintaining:
/*
void triangle(Vec2i& t0, Vec2i& t1, Vec2i& t2, TGAImage& image, const TGAColor& color) {
	//t0.y<=t2.y<=t3.y
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t1.y > t2.y) std::swap(t1, t2);
	if (t0.y > t2.y) std::swap(t0, t2);


	int total_height = t2.y - t0.y;
	//integration
	for (int i = 0; i <= total_height; ++i) {
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;

		float alpha = static_cast<float>(i) / total_height;   //be careful with division by zero
		float beta = static_cast<float>(i-(second_half?t1.y-t0.y: 0)) / segment_height; //be careful
		alpha = t0.x + (t2.x - t0.x)*alpha;
		beta = second_half ? t1.x + (t2.x - t1.x)*beta  : t0.x + (t1.x - t0.x)*beta;
		if (alpha > beta) std::swap(alpha, beta);
		for (int x = alpha; x <= beta; ++x) {
			image.set(x, t0.y+i, color);
		}
	}
}*/

Vec3f barycentric(Vec2i A, Vec2i B, Vec2i C, Vec2i P) {
	auto res = cross(Vec3f(B.x - A.x, C.x - A.x, A.x - P.x), Vec3f(B.y - A.y, C.y - A.y, A.y - P.y));
	//注意  如果res.z为0，这三个点不构成三角形
	if (std::abs(res.z) < 0.001f) return Vec3f(-1, 1, 1);
	return { 1.0f - (res.x + res.y) / res.z, res.y / res.z, res.x / res.z };
}

void triangle(Vec2i A, Vec2i B, Vec2i C, TGAImage& image, TGAColor color) {

	Vec2i bboxmin(std::max(0, std::min({ A.x,B.x,C.x })), std::max(0, std::min({ A.y,B.y,C.y })));
	//和0比较是为了防止超出屏幕
	Vec2i bboxmax(std::min(image.get_width()-1, std::max({ A.x,B.x,C.x })), std::min(image.get_height() - 1, std::max({ A.y,B.y,C.y })));

	Vec2i P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; ++P.x) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; ++P.y) {
			auto bc = barycentric(A, B, C, P);
			if (bc.x < 0 || bc.y < 0 || bc.z < 0)continue;
			image.set(P.x, P.y, color);
		}
	}
}

void test1() {
	TGAImage test(200, 200, TGAImage::RGB);

	triangle({ 10,10 }, { 100,30 }, { 190,160 }, test, TGAColor(255, 0, 0));
	test.flip_vertically();
	test.write_tga_file("Lesson2/triangle.tga");
}

void test2() {
	auto model = new Model("obj/african_head/african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);

	for (int i = 0; i < model->nfaces(); ++i) {
		auto face = model->face(i,0);
		Vec2i screen_coord[3];
		for (int j = 0; j < 3; ++j) {
			auto world_coord = model->vert(face[j]);
			screen_coord[j].x = (world_coord.x + 1.0f)*width / 2;
			screen_coord[j].y = (world_coord.y + 1.0f)*height / 2;
		}
		triangle(screen_coord[0], screen_coord[1], screen_coord[2], image, TGAColor(rand() % 255, rand() % 255, rand() % 255));
	}
	image.flip_vertically();
	image.write_tga_file("Lesson2/african.tga");
}

//添加光源
void test3() {
	auto model = new Model("obj/african_head/african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);
	Vec3f light_dir(0, 0, 1);

	for (int i = 0; i < model->nfaces(); ++i) {
		auto face = model->face(i, 0);
		Vec2i screen_coord[3];
		Vec3f world_coord[3];
		for (int j = 0; j < 3; ++j) {
			world_coord[j] = model->vert(face[j]);
			screen_coord[j].x = (world_coord[j].x + 1.0f)*width / 2;
			screen_coord[j].y = (world_coord[j].y + 1.0f)*height / 2;
		}
		auto normal = cross(world_coord[0] - world_coord[1], world_coord[0] - world_coord[2]);
		normal.normalize();//一定要注意单位化
		auto intensity = light_dir * normal;
		if(intensity > 0)
			triangle(screen_coord[0], screen_coord[1], screen_coord[2], image, TGAColor(intensity * 255, intensity * 255, intensity * 255));
	}
	image.flip_vertically();
	image.write_tga_file("Lesson2/african2.tga");

	delete model;
}

int main(){
	test3();
}	