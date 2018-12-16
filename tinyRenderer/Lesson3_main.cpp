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
#include <algorithm>
#include "tgaimage.h"
#include "geometry.h"
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const int width = 800;
const int height = 800;
Model* model = nullptr;

void line(Vec2i p0, Vec2i p1, TGAImage& image, TGAColor color) {
	bool steep = false;
	if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y)) {
		std::swap(p0.x, p0.y);
		std::swap(p1.x, p1.y);
		steep = true;
	}
	if (p0.x > p1.x) {
		std::swap(p0, p1);
	}
	float divisor = 1.0f / (float)(p1.x - p0.x);
	for (int x = p0.x; x <= p1.x; x++) {
		float t = (x - p0.x) * divisor;
		int y = p0.y*(1. - t) + p1.y*t + .5;
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
	}
}

void rasterize(Vec2i p0, Vec2i p1, TGAImage& image, TGAColor color, int ybuffer[]) {
	if (p0.x > p1.x)
		std::swap(p0, p1);
	float divisor = 1.0f / (float)(p1.x - p0.x);
	for (int x = p0.x; x <= p1.x; ++x) {
		float t = (x - p0.x) * divisor;
		int y = p0.y*(1. - t) + p1.y*t + .5;
		if (ybuffer[x] < y) {
			ybuffer[x] = y;
			image.set(x, 0, color);
		}
	}
}

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
	auto res = cross(Vec3f(B.x - A.x, C.x - A.x, A.x - P.x), Vec3f(B.y - A.y, C.y - A.y, A.y - P.y));
	if (std::abs(res.z) < 0.001f)
		return { -1,1,1 };
	return { 1.0f - (res.x + res.y) / res.z, res.x / res.z,res.y / res.z };
}
//注意重心坐标的返回顺序，要严格按照res计算顺序

void triangle(Vec3f A, Vec3f B, Vec3f C, TGAImage& image, TGAColor color, float zbuffer[]) {
	Vec2f bboxmin(std::max(0.0f, std::min({ A.x,B.x,C.x })), std::max(0.0f, std::min({ A.y,B.y,C.y })));
	//和0比较是为了防止超出屏幕
	Vec2f bboxmax(std::min(static_cast<float>(image.get_width() - 1), std::max({ A.x,B.x,C.x })), std::min(static_cast<float>(image.get_height() - 1), std::max({ A.y,B.y,C.y })));

	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; ++P.x) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; ++P.y) {
			auto bc = barycentric(A, B, C, P);
			if (bc.x < 0 || bc.y < 0 || bc.z < 0)continue;
			P.z = bc.x*A.z + bc.y*B.z + bc.z*C.z;
			if (zbuffer[int(P.x + P.y*width)] < P.z) {//int(P.x) + int(P.y)*width
				zbuffer[int(P.x + P.y*width)] = P.z;
				image.set(P.x, P.y, color);
			}			
		}
	}
}

//for uv
void triangleUV(Vec3f A, Vec3f B, Vec3f C, Vec2f* uv, TGAImage& image, float zbuffer[]) {
	Vec2f bboxmin(std::max(0.0f, std::min({ A.x,B.x,C.x })), std::max(0.0f, std::min({ A.y,B.y,C.y })));
	//和0比较是为了防止超出屏幕
	Vec2f bboxmax(std::min(static_cast<float>(image.get_width() - 1), std::max({ A.x,B.x,C.x })), std::min(static_cast<float>(image.get_height() - 1), std::max({ A.y,B.y,C.y })));

	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; ++P.x) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; ++P.y) {
			auto bc = barycentric(A, B, C, P);
			if (bc.x < 0 || bc.y < 0 || bc.z < 0)continue;
			P.z = bc.x*A.z + bc.y*B.z + bc.z*C.z;
			auto UV = uv[0] * bc.x + uv[1] * bc.y + uv[2] * bc.z;
			if (zbuffer[int(P.x + P.y*width)] < P.z) {//int(P.x) + int(P.y)*width
				zbuffer[int(P.x + P.y*width)] = P.z;
				image.set(P.x, P.y, model->diffuse(UV));
			}
		}
	}
}

void test1() {
	{ // just dumping the 2d scene (yay we have enough dimensions!)
		TGAImage scene(width, height, TGAImage::RGB);

		// scene "2d mesh"
		line(Vec2i(20, 34), Vec2i(744, 400), scene, red);
		line(Vec2i(120, 434), Vec2i(444, 400), scene, green);
		line(Vec2i(330, 463), Vec2i(594, 200), scene, blue);

		// screen line
		line(Vec2i(10, 10), Vec2i(790, 10), scene, white);

		scene.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		scene.write_tga_file("Lesson3/scene.tga");
	}
		TGAImage render(width, 16, TGAImage::RGB);
		int ybuffer[width];
		for (int i = 0; i < width; i++) {
			ybuffer[i] = std::numeric_limits<int>::min();
		}
		rasterize(Vec2i(20, 34), Vec2i(744, 400), render, red, ybuffer);
		rasterize(Vec2i(120, 434), Vec2i(444, 400), render, green, ybuffer);
		rasterize(Vec2i(330, 463), Vec2i(594, 200), render, blue, ybuffer);

		// 1-pixel wide image is bad for eyes, lets widen it
		//注意当j=0的时候，在rasterize已经设置过了
		for (int i = 0; i < width; i++) {
			for (int j = 1; j < 16; j++) {
				render.set(i, j, render.get(i, 0));
			}
		}
		render.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		render.write_tga_file("Lesson3/render3.tga");
}

void test2() {
	TGAImage image(width, height, TGAImage::RGB);
	auto model = new Model("obj/african_head/african_head.obj");
	float* zbuffer = new float[width*height];
	//注意当分配内存过大时，栈会溢出。。。 一定要用堆
	
	//这样写虽然简洁但是  一定记得加分号。。。。。。。。。。。
	for (int i = width * height; i--; zbuffer[i] = -FLT_MAX);
	
	Vec3f light_dir(0, 0, 1);
	for (int i = 0; i < model->nfaces(); ++i) {
		auto face = model->face(i, 0);
		Vec3f screen_coord[3];
		Vec3f tmp[3];
		int k = -1;
		for (auto e : face) {
			++k;
			tmp[k] = model->vert(e);
			screen_coord[k] = Vec3f(int((tmp[k].x + 1.0f)*width / 2.0f), int((tmp[k].y + 1.0f)*height / 2.0f), tmp[0].z);
		}
		auto norm = cross(tmp[0] - tmp[1], tmp[0] - tmp[2]);
		norm.normalize();
		auto intensity = norm * light_dir;
		intensity = intensity > 0 ? intensity : 0;
		triangle(screen_coord[0], screen_coord[1], screen_coord[2], image, TGAColor(255*intensity, 255 * intensity, 255 * intensity), zbuffer);
	}

	image.flip_vertically();
	image.write_tga_file("Lesson3/african.tga");
	delete model;
	delete[]zbuffer;
}

//添加纹理插值
void test3() {
	TGAImage image(width, height, TGAImage::RGB);
	model = new Model("obj/african_head/african_head.obj");
	float* zbuffer = new float[width*height];
	//注意当分配内存过大时，栈会溢出。。。 一定要用堆

	//这样写虽然简洁但是  一定记得加分号。。。。。。。。。。。
	for (int i = width * height; i--; zbuffer[i] = -FLT_MAX);

	Vec3f light_dir(0, 0, 1);
	//ver1
	/*
	for (int i = 0; i < model->nfaces(); ++i) {
		auto faceVer = model->face(i, 0);
		auto faceUV = model->face(i, 1);
		Vec3f screen_coord[3];
		Vec3f tmp[3];
		Vec2f UV[3];
		for (int i = 0; i < 3; ++i) {
			tmp[i] = model->vert(faceVer[i]);
			UV[i] = model->uv(faceUV[i]);
			screen_coord[i] = Vec3f(int((tmp[i].x + 1.0f)*width / 2.0f), int((tmp[i].y + 1.0f)*height / 2.0f), tmp[0].z);			
		}
		triangleUV(screen_coord[0], screen_coord[1], screen_coord[2], UV, image, zbuffer);
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
			UV[j] = model->uv(i,j);
			screen_coord[j] = Vec3f(int((tmp[j].x + 1.0f)*width / 2.0f), int((tmp[j].y + 1.0f)*height / 2.0f), tmp[0].z);
		}
		triangleUV(screen_coord[0], screen_coord[1], screen_coord[2], UV, image, zbuffer);
	}

	image.flip_vertically();
	image.write_tga_file("Lesson3/africanUV.tga");
	delete model;
	delete[]zbuffer;
}

int main(){
	test3();
}	