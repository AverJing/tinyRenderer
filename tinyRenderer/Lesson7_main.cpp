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
float* shadowbuffer;
Vec3f cameraPos(0, 0, 3);
Vec3f light_dir(1, 1, 0);
Vec3f eye(1, 1, 4);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);
const int depth = 255;

extern Matrix Projection;
extern Matrix Projection2;
extern Matrix Projection3;
extern Matrix LookAt;
extern Matrix ViewPort;
extern Matrix Translate;
extern Matrix OrthoProjection;

std::ofstream f_over("Lesson7/input8.txt");

inline Vec3f vec4Tovec3(Vec4f m, bool type) {
	//注意到这个细节，在OpenGL中perspective投影后其变成了左手系
	//从zNear到zFar深度值从-1到1
	//而此处和教程中相反，故而渲染后结果是反着的
	return Vec3f(int(m[0] / m[3]), int(m[1] / m[3]), type ? -(m[2] / m[3]) : (m[2] / m[3]));
}

inline Vec3f interpolation(Vec3f a, Vec3f b, Vec3f c, Vec3f barycentric) {
	return Vec3f((a.x*barycentric.x + b.x*barycentric.y + c.x*barycentric.z), (a.y*barycentric.x + b.y*barycentric.y + c.y*barycentric.z), (a.z*barycentric.x + b.z*barycentric.y + c.z*barycentric.z));
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

struct Shader : public IShader {
	mat<4, 4, float> uniform_M;   //  Projection*ModelView
	mat<4, 4, float> uniform_MIT; // (Projection*ModelView).invert_transpose()
	mat<4, 4, float> uniform_Mshadow; // transform framebuffer screen coordinates to shadowbuffer screen coordinates
	mat<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	mat<3, 3, float> varying_tri; // triangle coordinates before Viewport transform, written by VS, read by FS

	Shader(Matrix M, Matrix MIT, Matrix MS) : uniform_M(M), uniform_MIT(MIT), uniform_Mshadow(MS), varying_uv(), varying_tri() {}

	virtual Vec4f vertex(int iface, int nthvert) {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		Vec4f gl_Vertex = ViewPort * Projection3 * LookAt * embed<4>(model->vert(iface, nthvert));
		//varying_tri.set_col(nthvert, vec4Tovec3(gl_Vertex, false));//注意使用OpenGL变化的差别
		varying_tri.set_col(nthvert, model->vert(iface, nthvert));
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor &color) {
		//Vec4f sb_p = uniform_Mshadow * embed<4>(varying_tri*bar); // corresponding point in the shadow buffer
		Vec3f sb_p1 = vec4Tovec3(uniform_Mshadow * embed<4>(varying_tri.col(0)), true);
		Vec3f sb_p2 = vec4Tovec3(uniform_Mshadow * embed<4>(varying_tri.col(1)), true);
		Vec3f sb_p3 = vec4Tovec3(uniform_Mshadow * embed<4>(varying_tri.col(2)), true);
		auto sb_p = interpolation(sb_p1, sb_p2, sb_p3, bar);
		//sb_p = sb_p / sb_p[3];
		f_over << sb_p << std::endl;
		int idx = int(sb_p[0]) + int(sb_p[1])*width; // index in the shadowbuffer array
		float shadow = (shadowbuffer[idx] < sb_p[2]-0.005);
		Vec2f uv = varying_uv * bar;                 // interpolate uv for the current pixel
		Vec3f n = proj<3>(uniform_MIT*embed<4>(model->normal(uv))).normalize(); // normal
		Vec3f l = proj<3>(uniform_M  *embed<4>(light_dir)).normalize(); // light vector
		Vec3f r = (n*(n*l*2.f) - l).normalize();   // reflected light
		float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
		float diff = std::max(0.f, n*l);
		TGAColor c = model->diffuse(uv);
		//for (int i = 0; i < 3; i++) color[i] = std::min<float>(20 + c[i] * shadow *(1.2*diff + .6*spec), 255);
		color = c * (0.3 + shadow) * (spec + diff);
		return false;
	}
};

struct DepthShader : public IShader {
	mat<3, 3, float> varying_tri;//保存这个矩阵，就是为了在fragment中计算color
	
	Vec4f vertex(int iface, int nthervt) override {//virtual
		//问题出现在LookAt矩阵上
		//如果使用glm的LookAt矩阵，最终z的范围不再0-255之间。
		auto tmp = OrthoProjection * LookAt * embed<4>(model->vert(iface, nthervt));
		auto gl_vertex = ViewPort * tmp;
		varying_tri.set_col(nthervt, vec4Tovec3(gl_vertex, true));
		return gl_vertex;	//w=1.0f												
	}

	bool fragment(Vec3f bar, TGAColor& color) override {
		auto p = varying_tri * bar;
		color = TGAColor(255, 255, 255)*(std::abs(p.z) / 255.0f);
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

void test7_1() {
	std::ofstream f("Lesson7/input3.txt");
	TGAImage image(width, height, TGAImage::RGB);
	model = new Model("obj/african_head/african_head.obj");

	//问题1：用教程中的LookAt最终viewport数据z的值确实在0-255之间
	//但是glm的LookAt最终viewport数据z值很多大于255，这就导致了很多细节丢失。
	//https://learnopengl-cn.github.io/05%20Advanced%20Lighting/03%20Shadows/01%20Shadow%20Mapping/#
	//此处是用正射投影来解决的。

	//个人觉得，是因为glm的使物体发生了位移故而会导致最终viewport中的z变化，必须通过投影矩阵处理
	//而教程中的center是Vec3f(0.0f, 0.0f, 0.0f)，只是换系，却不发生位移
	projection4(0.0f);
	projection3(PI / 4.0f, 1.0f, 0.1f, 100.0f);
	lookat(light_dir, Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 1.0f, 0.0f));
	//viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	viewport(0, 0, width, height);
	translate(Vec3f(0.0f, 0.0f, 1.0f));
	ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 3.0f);//效果好一点

	shadowbuffer = new float[width*height];
	for (int i = 0; i < width*height; i++) {
		shadowbuffer[i] = std::numeric_limits<int>::min();
	}

	DepthShader shader;
	light_dir.normalize();

	for (int i = 0; i < model->nfaces(); ++i) {
		Vec3f screen_coords[3];
		for (int j = 0; j < 3; ++j) {
			screen_coords[j] = vec4Tovec3(shader.vertex(i, j), true); //记得剪裁
			f << screen_coords[j] << ' ';
		}
		f << std::endl;
		triangle(screen_coords[0], screen_coords[1], screen_coords[2], shader, image, shadowbuffer);
	}
	image.flip_vertically();
	image.write_tga_file("Lesson7/shadowbuffer2.tga");

	delete model;
	delete[] shadowbuffer;
}

//此处glm版本暂时没修正
//作者提供地版本  eye只要方向不变，那么生成地深度贴图没变化
//但glm版本地就不一样了。只要eye发生变化，深度贴图结果跟着发生变化。
void test7_2() {
	std::ofstream f("Lesson7/input2.txt");
	float *zbuffer = new float[width*height];
	shadowbuffer = new float[width*height];
	for (int i = width * height; --i; ) {
		zbuffer[i] = shadowbuffer[i] = -std::numeric_limits<float>::max();
	}

	model = new Model("obj/african_head/african_head.obj");
	//light_dir.normalize();

	//test7_1
	{ // rendering the shadow buffer
		TGAImage depth(width, height, TGAImage::RGB);
		lookat(light_dir, center, up);
		viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		projection4(0);
		projection3(PI / 4, 1.0f, 0.1f, 100.0f);
		translate(Vec3f(0.0f, 0.0f, 1.0f));
		ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 3.0f);

		DepthShader depthshader;
		Vec3f screen_coords[3];
		for (int i = 0; i < model->nfaces(); i++) {
			for (int j = 0; j < 3; j++) {
				screen_coords[j] = vec4Tovec3(depthshader.vertex(i, j), true);
				f << screen_coords[j];
			}
			f << std::endl;
			triangle(screen_coords[0], screen_coords[1], screen_coords[2], depthshader, depth, shadowbuffer);
		}
		depth.flip_vertically(); // to place the origin in the bottom left corner of the image
		depth.write_tga_file("Lesson7/depth.tga");
	}

	Matrix M = ViewPort * OrthoProjection * LookAt;

	{ // rendering the frame buffer
		TGAImage frame(width, height, TGAImage::RGB);
		lookat(eye, center, up);
		viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		projection4(-1.0f/(eye - center).norm());
		projection3(PI / 4.0f, 1.0f, 0.1f, 100.0f);
		//这边其实可以直接从世界坐标变化，而不需要求逆
		Shader shader(LookAt, (Projection3*LookAt).invert_transpose(), M);//*(ViewPort*Projection3*LookAt).invert()
		Vec3f screen_coords[3];
		for (int i = 0; i < model->nfaces(); i++) {
			for (int j = 0; j < 3; j++) {
				screen_coords[j] = vec4Tovec3(shader.vertex(i, j), true);
			}
			triangle(screen_coords[0], screen_coords[1], screen_coords[2], shader, frame, zbuffer);
		}
		frame.flip_vertically(); // to place the origin in the bottom left corner of the image
		frame.write_tga_file("Lesson7/framebuffer.tga");
	}

	delete model;
	delete[] zbuffer;
	delete[] shadowbuffer;
}

int main() {
	//test7_1();
	test7_2();
}