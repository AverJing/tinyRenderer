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
#include <algorithm>
#include <cmath>
#include <fstream>
#include <vector>
#include "tgaimage.h"
#include "model.h"
#include "our_gl.h"

//Lesson5部分和Lesson6整合
using std::vector;

TGAColor white(255, 255, 255);
TGAColor green(0, 255, 0);
TGAColor red(255, 0, 0);
TGAColor blue(0, 0, 255);
TGAColor yellow(255, 255, 0);

const int width = 800;
const int height = 800;

const float PI = 3.1415926f;
Model* model = nullptr;
Vec3f cameraPos(0, 0, 3);
Vec3f light_dir(3, 3, 0);
const int depth = 255;

extern Matrix Projection;
extern Matrix Projection2;
extern Matrix Projection3;
extern Matrix LookAt;
extern Matrix ViewPort;
extern Matrix Translate;

Vec3f vec4Tovec3(Vec4f m) {
	//注意到这个细节，在OpenGL中perspective投影后其变成了左手系
	//从zNear到zFar深度值从-1到1
	//而此处和教程中相反，故而渲染后结果是反着的
	return Vec3f(int(m[0] / m[3]), int(m[1] / m[3]), -int(m[2] / m[3]));
}

void triangle(Vec3f A, Vec3f B, Vec3f C, TGAImage& image, float zbuffer[]) {
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
				image.set(P.x, P.y, white);
			}
		}
	}
}

//specular+diffuse
struct Shader : public IShader{
	mat<2, 3, float> varying_uv;
	Matrix  uniform_MIT;  //Projection * View .invert_transpose
	Matrix  uniform_M;	//Projection * View
	Vec4f FragPos;
	Vec4f vertex(int iface, int nthervt) override{//virtual
		varying_uv.set_col(nthervt, model->uv(iface, nthervt));
		FragPos = embed<4>(model->vert(iface, nthervt));
		return ViewPort * uniform_M * FragPos;	//w=1.0f												
	}

	bool fragment(Vec3f bar, TGAColor& color) override {
		auto uv = varying_uv * bar;
		//应该在世界坐标下进行
		auto n = proj<3>( (embed<4>(model->normal(uv)))).normalize();
		auto l = proj<3>( (embed<4>(light_dir))).normalize();

		auto r = (n*(n*l*2.0f) - l).normalize();
		//假定摄像机位置就是眼的位置
		//cameraPos-FragPos 是指观察向量
		//注意此处的应该使用全局坐标的法线贴图。
		//Vec3f(1.0f, 1.0f, 3.0f)为摄像机eye向量
		float spec = std::pow(std::max(proj<3>(embed<4>(Vec3f(1.0f, 1.0f, 3.0f)) - FragPos).normalize() * r, 0.0f), model->specular(uv));
		//float spec = std::pow(std::max(r.z, 0.0f), model->specular(uv));
		float diff = std::max(0.0f, n*l);
		color = model->diffuse(uv);
		color = color * (0.4*spec + 0.4*diff + 0.2f);//最好使用*=，此处没有重载
		//for (int i = 0; i < 3; i++) color[i] = std::min<float>(color[i] *(0.2 + diff + 0.6*spec), 255);
		return false;
	}
};

//specular+diffuse
struct EasyShader : public IShader {
	mat<2, 3, float> varying_uv;
	Vec4f vertex(int iface, int nthervt) override {//virtual
		varying_uv.set_col(nthervt, model->uv(iface, nthervt));
		return ViewPort * Projection3 * LookAt * Translate * embed<4>(model->vert(iface, nthervt));	//w=1.0f												
	}

	bool fragment(Vec3f bar, TGAColor& color) override {
		auto uv = varying_uv * bar;
		color = model->diffuse(uv);
		return false;
	}
};

void triangle(Vec3f A, Vec3f B, Vec3f C, IShader& shader, TGAImage& image, float zbuffer[]) {
	Vec2f bboxmin(std::max(0.0f, std::min({ A.x,B.x,C.x })), std::max(0.0f, std::min({ A.y,B.y,C.y })));
	//和0比较是为了防止超出屏幕
	Vec2f bboxmax(std::min(static_cast<float>(image.get_width() - 1), std::max({ A.x,B.x,C.x })), std::min(static_cast<float>(image.get_height() - 1), std::max({ A.y,B.y,C.y })));

	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; ++P.x) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; ++P.y) {
			auto bc = barycentric(A, B, C, P);
			P.z = bc.x*A.z + bc.y*B.z + bc.z*C.z;
			if (bc.x < 0 || bc.y < 0 || bc.z < 0|| zbuffer[int(P.x + P.y*width)] > P.z)continue;
			TGAColor color;
			bool discard = shader.fragment(bc, color);
			if (!discard) {//int(P.x) + int(P.y)*width
				zbuffer[int(P.x + P.y*width)] = P.z;
				image.set(P.x, P.y, color);
			}

		}
	}
}

//测试一个正方体的深度
void test6_1() {
	std::ofstream f("Lesson6/input2.txt");
	TGAImage image(width, height, TGAImage::RGB);
	float* zbuffer = new float[width*height];
	//注意当分配内存过大时，栈会溢出。。。 一定要用堆

	//这样写虽然简洁但是  一定记得加分号。。。。。。。。。。。
	for (int i = width * height; i--; zbuffer[i] = -FLT_MAX);

	viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	projection(cameraPos.z);
	projection3(PI/4, 1.0f, 0.1f, 100.0f);
	lookat(Vec3f(0.0f, 0.0f, 3.0f), Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 1.0f, 0.0f));

	//quad顶点数据
	vector<vector<float>> quad_coords{ 
		//后
		{-0.5f, -0.5f, -0.5f},
		{0.5f, -0.5f, -0.5f},
		{0.5f,  0.5f, -0.5f},
		{0.5f,  0.5f, -0.5f},
		{-0.5f,  0.5f, -0.5f},
		{-0.5f, -0.5f, -0.5f},
		//前
		{-0.5f, -0.5f,  0.5f},
		{0.5f, -0.5f,  0.5f},
		{0.5f,  0.5f,  0.5f},
		{0.5f,  0.5f,  0.5f},
		{-0.5f,  0.5f,  0.5f},
		{-0.5f, -0.5f,  0.5f},

		{-0.5f,  0.5f,  0.5f},
		{-0.5f,  0.5f, -0.5f},
		{-0.5f, -0.5f, -0.5f},
		{-0.5f, -0.5f, -0.5f},
		{-0.5f, -0.5f,  0.5f},
		{-0.5f,  0.5f,  0.5f},

		{0.5f,  0.5f,  0.5f},
		{0.5f,  0.5f, -0.5f},
		{0.5f, -0.5f, -0.5f},
		{0.5f, -0.5f, -0.5f},
		{0.5f, -0.5f,  0.5f},
		{0.5f,  0.5f,  0.5f},

		{-0.5f, -0.5f, -0.5f},
		{0.5f, -0.5f, -0.5f},
		{0.5f, -0.5f,  0.5f},
		{0.5f, -0.5f,  0.5f},
		{-0.5f, -0.5f,  0.5f},
		{-0.5f, -0.5f, -0.5f},

		{-0.5f,  0.5f, -0.5f},
		{0.5f,  0.5f, -0.5f},
		{0.5f,  0.5f,  0.5f},
		{0.5f,  0.5f,  0.5f},
		{-0.5f,  0.5f,  0.5f},
		{-0.5f,  0.5f, -0.5f}
	};

	for (int i = 0; i < quad_coords.size(); i+=3) {
		Vec3f screen_coord[3];
		for (int j = 0; j < 3; ++j) {
			screen_coord[j] = vec4Tovec3(ViewPort * Projection * LookAt * rotateByYaxis(PI/4) *
				Vec4f(quad_coords[i+j][0], quad_coords[i + j][1], quad_coords[i + j][2], 1.0f));
			f << screen_coord[j];
		}
		f << std::endl;
		triangle(screen_coord[0], screen_coord[1], screen_coord[2], image, zbuffer);
	}

	image.flip_vertically();
	image.write_tga_file("Lesson6/quad.tga");

	// dump z-buffer (debugging purposes only)
	TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			zbimage.set(i, j, TGAColor(zbuffer[i + j * width]));
		}
	}
	zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	zbimage.write_tga_file("Lesson6/zbufferQuad.tga");

	delete[]zbuffer;
	f.close();
}

void test6_2() {
	TGAImage image(width, height, TGAImage::RGB);
	model = new Model("obj/african_head/african_head.obj");

	projection(cameraPos.z);
	projection3(PI / 4.0f, 1.0f, 0.1f, 100.0f);
	lookat(Vec3f(1.0f, 1.0f, 3.0f), Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 1.0f, 0.0f));
	viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	translate(Vec3f(0.0f, 0.0f, 1.0f));

	auto zbuffer = new float[width*height];
	for (int i = 0; i < width*height; i++) {
		zbuffer[i] = std::numeric_limits<int>::min();
	}

	Shader shader;
	//这两个其实没什么用
	shader.uniform_M = Projection3 * LookAt * Translate;
	shader.uniform_MIT = (shader.uniform_M).invert_transpose();
	light_dir.normalize();

	for (int i = 0; i < model->nfaces(); ++i) {
		Vec3f screen_coords[3];
		for (int j = 0; j < 3; ++j) {
			screen_coords[j] = vec4Tovec3(shader.vertex(i, j)); //记得剪裁
		}
		triangle(screen_coords[0], screen_coords[1], screen_coords[2], shader, image, zbuffer);
	}
	image.flip_vertically();
	image.write_tga_file("Lesson6/african.tga");

	{
		TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				zbimage.set(i, j, TGAColor(zbuffer[i + j * width]));
			}
		}
		zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		zbimage.write_tga_file("Lesson6/zbuffer2.tga");
	}

	delete model;
	delete[] zbuffer;
}

void test6_3() {
	std::ofstream f("Lesson6/test6_3_output.txt");
	TGAImage image(width, height, TGAImage::RGB);
	model = new Model("obj/african_head/african_head.obj");

	projection(cameraPos.z);
	//为什么用Projection3  正好是背面？
	//会不会是模型的问题？
	projection3(PI/4, 1.0f, 0.1f, 100.0f);
	lookat(Vec3f(0.0f, 0.0f, 3.0f), Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 1.0f, 0.0f));
	//viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	//viewport(0, 0, width, height);
	translate(Vec3f(0.0f, 0.0f, 1.0f));

	auto zbuffer = new float[width*height];
	for (int i = 0; i < width*height; i++) {
		zbuffer[i] = std::numeric_limits<int>::min();
	}

	EasyShader shader;
	light_dir.normalize();

	for (int i = 0; i < model->nfaces(); ++i) {
		Vec3f screen_coords[3];
		for (int j = 0; j < 3; ++j) {
			screen_coords[j] = vec4Tovec3(shader.vertex(i, j)); //记得剪裁
			f << screen_coords[j];
		}
		f << std::endl;
		triangle(screen_coords[0], screen_coords[1], screen_coords[2], shader, image, zbuffer);
	}
	image.flip_vertically();
	image.write_tga_file("Lesson6/african3.tga");

	{
		TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				zbimage.set(i, j, TGAColor(zbuffer[i + j * width]));
			}
		}
		zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		zbimage.write_tga_file("Lesson6/zbuffer5.tga");
	}

	delete model;
	delete[] zbuffer;
}

int main() {
	//test6_1();

	//test6_2();
	test6_3();
}