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
#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
//#include "line.h"
//#include "triangle.h"
#include "our_gl.h"

Model *model = nullptr;
float *shadowbuffer = nullptr;

const int width = 800;
const int height = 800;
const float M_PI = 3.1415926;

Vec3f       eye(1.2, -.8, 3);
Vec3f    center(0, 0, 0);
Vec3f        up(0, 1, 0);

TGAImage total(1024, 1024, TGAImage::GRAYSCALE);
TGAImage  occl(1024, 1024, TGAImage::GRAYSCALE);

extern Matrix ModelView;
extern Matrix ViewPort;
extern Matrix Projection;

struct ZShader : public IShader {
	mat<4, 3, float> varying_tri;

	virtual Vec4f vertex(int iface, int nthvert) {
		Vec4f gl_Vertex = Projection * ModelView*embed<4>(model->vert(iface, nthvert));
		varying_tri.set_col(nthvert, gl_Vertex);
		return gl_Vertex;
	}

	bool fragment(Vec3f bar, TGAColor& color) { return false; }

	//for Jcy_Lesson8_ver1
	/*
	virtual bool fragment(Vec3f gl_FragCoord, Vec3f bar, TGAColor &color) override{
		color = TGAColor(255, 255, 255)*((gl_FragCoord.z + 1.f) / 2.f);
		return false;
	}*/
	//for Jcy_Lesson8_ver2
	virtual bool fragment(Vec3f gl_FragCoord, Vec3f bar, TGAColor &color) override {
		color = TGAColor(0, 0, 0);
		return false;
	}
};

struct Shader : public IShader {
	mat<2, 3, float> varying_uv;
	mat<4, 3, float> varying_tri;

	virtual Vec4f vertex(int iface, int nthvert) {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		Vec4f gl_Vertex = Projection * ModelView* embed<4>(model->vert(iface, nthvert));
		varying_tri.set_col(nthvert, gl_Vertex);
		return gl_Vertex;
	}

	bool fragment(Vec3f bar, TGAColor& color) { return false; }

	virtual bool fragment(Vec3f gl_FragCoord, Vec3f bar, TGAColor &color) override {
		Vec2f uv = varying_uv * bar;
		if (std::abs(shadowbuffer[int(gl_FragCoord.x + gl_FragCoord.y*width)] - gl_FragCoord.z < 1e-2)) {
			occl.set(uv.x * 1024, uv.y * 1024, TGAColor(255));
		}
		color = TGAColor(255, 0, 0);
		return false;
	}
};

struct AOShader : public IShader {
	mat<2, 3, float> varying_uv;
	mat<4, 3, float> varying_tri;
	TGAImage aoimage;

	virtual Vec4f vertex(int iface, int nthvert) {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		Vec4f gl_Vertex = Projection * ModelView*embed<4>(model->vert(iface, nthvert));
		varying_tri.set_col(nthvert, gl_Vertex);
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f gl_FragCoord, Vec3f bar, TGAColor &color) {
		Vec2f uv = varying_uv * bar;
		int t = aoimage.get(uv.x * 1024, uv.y * 1024)[0];
		color = TGAColor(t, t, t);
		return false;
	}
};

//计算抛出光线的最大斜率
float max_elevation_angle(float* zbuffer, Vec2f p, Vec2f dir) {
	float maxangle = 0.0f;
	for (float t = 0.0f; t < 100.0f; t += 1.0f) {
		Vec2f cur = p + dir * t;
		if (cur.x >= width || cur.y >= height || cur.x < 0 || cur.y < 0) return maxangle;

		float dist = (p - cur).norm();
		if (dist < 1.0f) continue;
		float elevation = zbuffer[int(cur.x) + int(cur.y)*width] - zbuffer[int(p.x) + int(p.y)*width];
		maxangle = std::max(maxangle, atanf(elevation / dist));
	}
	return maxangle;
}


//单位球
Vec3f rand_point_on_unit_sphere() {
	float u = (float)rand() / (float)RAND_MAX;
	float v = (float)rand() / (float)RAND_MAX;
	float theta = 2.f*M_PI*u;
	float phi = acos(2.f*v - 1.f);
	return Vec3f(sin(phi)*cos(theta), sin(phi)*sin(theta), cos(phi));
}//看笔记，

void Jcy_Lesson8_ver1() {
	float *zbuffer = new float[width*height];
	shadowbuffer = new float[width*height];
	model = new Model("obj/diablo3_pose/diablo3_pose.obj");

	//TGAImage frame(width, height, TGAImage::RGB);
	//lookat(eye, center, up);
	//viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	//projection(-1.f / (eye - center).norm());
	for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

	/*
	AOShader aoshader;
	aoshader.aoimage.read_tga_file("occlusion.tga");
	aoshader.aoimage.flip_vertically();
	for (int i=0; i<model->nfaces(); i++) {
		for (int j=0; j<3; j++) {
			aoshader.vertex(i, j);
		}
		triangle(aoshader.varying_tri, aoshader, frame, zbuffer);
	}
	frame.flip_vertically();
	frame.write_tga_file("framebuffer.tga");
	return 0;
	*/

	const int nrenders = 10;
	for (int iter = 1; iter <= nrenders; iter++) {
		std::cerr << iter << " from " << nrenders << std::endl;
		for (int i = 0; i < 3; i++) up[i] = (float)rand() / (float)RAND_MAX;
		eye = rand_point_on_unit_sphere();
		eye.y = std::abs(eye.y);//上半球
		std::cout << "v " << eye << std::endl;

		for (int i = width * height; i--; shadowbuffer[i] = zbuffer[i] = -std::numeric_limits<float>::max());

		TGAImage frame(width, height, TGAImage::RGB);
		lookat(eye, center, up);
		viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		projection(0);//-1.f/(eye-center).norm());
		//只是为了求z-buffer
		ZShader zshader;
		for (int i = 0; i < model->nfaces(); i++) {
			for (int j = 0; j < 3; j++) {
				zshader.vertex(i, j);
			}
			triangle(zshader.varying_tri, zshader, frame, shadowbuffer);
		}
		frame.flip_vertically();
		frame.write_tga_file("framebuffer.tga");
		Shader shader;
		occl.clear();
		for (int i = 0; i < model->nfaces(); i++) {
			for (int j = 0; j < 3; j++) {
				shader.vertex(i, j);
			}
			triangle(shader.varying_tri, shader, frame, zbuffer);
		}

		//        occl.gaussian_blur(5);
		for (int i = 0; i < 1024; i++) {
			for (int j = 0; j < 1024; j++) {
				float tmp = total.get(i, j)[0];
				total.set(i, j, TGAColor((tmp*(iter - 1) + occl.get(i, j)[0]) / (float)iter + .5f));
				//取均值
			}
		}
	}
	total.flip_vertically();
	total.write_tga_file("occlusion.tga");
	occl.flip_vertically();
	occl.write_tga_file("occl.tga");

	delete[] zbuffer;
	delete model;
	delete[] shadowbuffer;
}

void Jcy_Lesson8_ver2() {
	float* zbuffer = new float[width*height];
	for (int i = width * height - 1; i--; zbuffer[i] = -FLT_MAX); //
	model = new Model("obj/african_head/african_head.obj");

	TGAImage frame(width, height, TGAImage::RGB);
	lookat(eye, center, up);
	viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	projection(-1.0f / (eye - center).norm());

	ZShader zshader;
	for (int i = 0; i < model->nfaces(); ++i) {
		for (int j = 0; j < 3; ++j) {
			zshader.vertex(i, j);
		}
		triangle(zshader.varying_tri, zshader, frame, zbuffer);
	}

	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			if (zbuffer[x + y * width] < -1e-4) continue;
			float total = 0.0f;
			for (float a = 0.0f; a < M_PI * 2 - 1e-4; a += M_PI / 4) {
				total += M_PI / 2 - max_elevation_angle(zbuffer, Vec2f(x, y), Vec2f(cos(a), sin(a)));
				//如果maxangle是90° 那么意味着它在某个“峡谷”下边，反射很少的光
			}
			total /= (M_PI / 2) * 8;
			total = std::pow(total, 100.0f);
			frame.set(x, y, TGAColor(255 * total, 255 * total, 255 * total));
		}
	}

	frame.flip_vertically();
	frame.write_tga_file("framebuffer.tga");

	delete[] zbuffer;
	delete model;
}

int main() {
	Jcy_Lesson8_ver2();
}