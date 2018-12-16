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
Vec3f light_dir(1, 1, 1);
const int depth = 255;

extern Matrix Projection;
extern Matrix Projection2;
extern Matrix Projection3;
extern Matrix LookAt;
extern Matrix ViewPort;
extern Matrix Translate;

Vec3f vec4Tovec3(Vec4f m, bool type) {
	//注意到这个细节，在OpenGL中perspective投影后其变成了左手系
	//从zNear到zFar深度值从-1到1
	//而此处和教程中相反，故而渲染后结果是反着的
	return Vec3f(int(m[0] / m[3]), int(m[1] / m[3]), type? -int(m[2] / m[3]) : int(m[2] / m[3]));
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

//use tangent coordinate
//frame:
//切线空间，就是为了应对物体会发生变化的情况。
struct Shader : public IShader {
	mat<2, 3, float> varying_uv;
	mat<3, 3, float> varying_nrm;
	mat<3, 3, float> ndc_pos;//ndc后的三角形顶点坐标
	//我直接在世界坐标下切换到切线空间，在shaderWorld着色器实现
	Vec4f vertex(int iface, int nthervt) override {//virtual
		varying_uv.set_col(nthervt, model->uv(iface, nthervt));
		varying_nrm.set_col(nthervt, proj<3>((ViewPort * Projection*LookAt).invert_transpose() * embed<4>(model->normal(iface, nthervt), 1.0f)));
		auto gl_vertex = ViewPort * Projection * LookAt * embed<4>(model->vert(iface, nthervt));//默认w=1.0f
		ndc_pos.set_col(nthervt, vec4Tovec3(gl_vertex, false));
		return embed<4>(ndc_pos.col(nthervt));	//标准化后的												
	}

	bool fragment(Vec3f bar, TGAColor& color) override {
		auto uv = varying_uv * bar;
		auto bn = (varying_nrm * bar).normalize();
		
		mat<3, 3, float> A;
		A[0] = ndc_pos.col(1)- ndc_pos.col(0);
		A[1] = ndc_pos.col(2) - ndc_pos.col(0);
		A[2] = bn;

		auto AI = A.invert();
		auto i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
		auto j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);

		mat<3, 3, float> B;
		B.set_col(0, i.normalize());
		B.set_col(1, j.normalize());
		B.set_col(2, bn);

		auto n = B * (model->normal(uv)).normalize();
		color = model->diffuse(uv) * std::max(0.0f, n*light_dir);

		return false;
	}
};

struct ShaderWorld : public IShader {
	mat<2, 3, float> varying_uv;
	mat<3, 3, float> varying_nrm;
	mat<3, 3, float> world_pos;//ndc后的三角形顶点坐标
	//我直接在世界坐标下切换到切线空间，在shaderWorld着色器实现
	Vec4f vertex(int iface, int nthervt) override {//virtual
		varying_uv.set_col(nthervt, model->uv(iface, nthervt));
		varying_nrm.set_col(nthervt, model->normal(iface, nthervt));
		auto gl_vertex = embed<4>(model->vert(iface, nthervt));//默认w=1.0f
		world_pos.set_col(nthervt, proj<3>(gl_vertex));
		return ViewPort * Projection3 * LookAt * Translate * gl_vertex;	//标准化后的												
	}

	bool fragment(Vec3f bar, TGAColor& color) override {
		auto uv = varying_uv * bar;
		auto bn = (varying_nrm * bar).normalize();

		mat<3, 3, float> A;
		A[0] = world_pos.col(1) - world_pos.col(0);
		A[1] = world_pos.col(2) - world_pos.col(0);
		A[2] = bn;

		auto AI = A.invert();
		auto i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
		auto j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);

		mat<3, 3, float> B;
		B.set_col(0, i.normalize());
		B.set_col(1, j.normalize());
		B.set_col(2, bn);

		auto n = B * (model->normal(uv)).normalize();
		color = model->diffuse(uv) * std::max(0.0f, n*light_dir);

		return false;
	}
};

//specular+diffuse
struct EasyShader : public IShader {
	mat<2, 3, float> varying_uv;
	Vec4f vertex(int iface, int nthervt) override {//virtual
		varying_uv.set_col(nthervt, model->uv(iface, nthervt));
		return ViewPort * Projection * LookAt * embed<4>(model->vert(iface, nthervt));	//w=1.0f												
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
			if (bc.x < 0 || bc.y < 0 || bc.z < 0 || zbuffer[int(P.x + P.y*width)] > P.z)continue;
			TGAColor color;
			bool discard = shader.fragment(bc, color);
			if (!discard) {//int(P.x) + int(P.y)*width
				zbuffer[int(P.x + P.y*width)] = P.z;
				image.set(P.x, P.y, color);
			}
		}
	}
}

void test6bis_1() {
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
	light_dir.normalize();

	for (int i = 0; i < model->nfaces(); ++i) {
		Vec3f screen_coords[3];
		for (int j = 0; j < 3; ++j) {
			screen_coords[j] = proj<3>(shader.vertex(i, j)); //记得剪裁
		}
		triangle(screen_coords[0], screen_coords[1], screen_coords[2], shader, image, zbuffer);
	}
	image.flip_vertically();
	image.write_tga_file("Lesson6bis/african.tga");

	{
		TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				zbimage.set(i, j, TGAColor(zbuffer[i + j * width]));
			}
		}
		zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		zbimage.write_tga_file("Lesson6bis/zbuffer2.tga");
	}

	delete model;
	delete[] zbuffer;
}

//和test6bis_1一样 不过使用的shaderworld着色器
//更推荐
//注意切线是tangent
void test6bis_2() {
	std::ofstream f("Lesson6bis/input_test6bis_2.txt");
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

	ShaderWorld shader;
	light_dir.normalize();

	for (int i = 0; i < model->nfaces(); ++i) {
		Vec3f screen_coords[3];
		for (int j = 0; j < 3; ++j) {
			screen_coords[j] = vec4Tovec3(shader.vertex(i, j), true); //记得剪裁
			f << screen_coords[j];
		}
		f << std::endl;
		triangle(screen_coords[0], screen_coords[1], screen_coords[2], shader, image, zbuffer);
	}
	image.flip_vertically();
	image.write_tga_file("Lesson6bis/african2.tga");

	{
		TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				zbimage.set(i, j, TGAColor(zbuffer[i + j * width]));
			}
		}
		zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		zbimage.write_tga_file("Lesson6bis/zbuffer2.tga");
	}

	delete model;
	delete[] zbuffer;
}

void test6_3() {
	TGAImage image(width, height, TGAImage::RGB);
	model = new Model("obj/african_head/african_head.obj");

	projection(cameraPos.z);
	projection3(PI / 4, 1.0f, 0.1f, 100.0f);
	lookat(Vec3f(0.0f, 0.0f, 3.0f), Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 1.0f, 0.0f));
	viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
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
			screen_coords[j] = vec4Tovec3(shader.vertex(i, j), false); //记得剪裁
		}
		triangle(screen_coords[0], screen_coords[1], screen_coords[2], shader, image, zbuffer);
	}
	image.flip_vertically();
	image.write_tga_file("Lesson6bis/african3.tga");

	{
		TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				zbimage.set(i, j, TGAColor(zbuffer[i + j * width]));
			}
		}
		zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		zbimage.write_tga_file("Lesson6bis/zbuffer4.tga");
	}

	delete model;
	delete[] zbuffer;
}

int main() {
	//test6bis_1();
	test6bis_2();
	//test6_3();
}