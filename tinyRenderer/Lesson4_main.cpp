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
#include "tgaimage.h"
#include "model.h"

TGAColor white(255, 255, 255);
TGAColor green(0, 255, 0);
TGAColor red(255, 0, 0);
TGAColor blue(0, 0, 255);
TGAColor yellow(255, 255, 0);

const int width = 800;
const int height = 800;

using Matrix2_2 = mat<2, 2, float>;
const float PI = 3.1415926f;
Model* model = nullptr;
Vec3f cameraPos(0, 0, 3);
Vec3f light_dir(0, 0, -1);
const int depth = 255;

void line5(const Vec2f& t0, const Vec2f& t1, TGAImage& image, const TGAColor& color) {
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

inline Matrix2_2 scale(float size) {
	Matrix2_2 tmp;
	tmp[0][0] = size;
	tmp[1][1] = size;
	return tmp;
}

inline Matrix2_2 shearing(float size) {
	Matrix2_2 tmp = Matrix2_2::identity();
	tmp[0][1] = size;
	return tmp;
}

inline Matrix2_2 rotate(float theta) {
	Matrix2_2 tmp;
	tmp[0][0] = cos(theta);
	tmp[0][1] = -sin(theta);
	tmp[1][0] = sin(theta);
	tmp[1][1] = cos(theta);

	return tmp;
}

//2D plane
void test4_1() {
	TGAImage image(400, 400, TGAImage::RGB);

	Vec2f a(100, 100);
	Vec2f b(300, 100);
	Vec2f c(300, 200);
	Vec2f d(200, 300);
	Vec2f e(100, 300);
	Vec2f o(200, 200);

	line5(a, b, image, white);
	line5(b, c, image, white);
	line5(c, d, image, white);
	line5(d, e, image, white);
	line5(e, a, image, white);

	line5(o, c, image, red);
	line5(o, d, image, green);

	//注意题目是以o为中心的
	/*line5(a*3/2, b * 3 / 2, image, blue);
	line5(b * 3 / 2, c * 3 / 2, image, blue);
	line5(c * 3 / 2, d * 3 / 2, image, blue);
	line5(d * 3 / 2, e * 3 / 2, image, blue);
	line5(e * 3 / 2, a * 3 / 2, image, blue);*/

	//o is center
	line5(a * 3 / 2 - o/2, b * 3 / 2 - o / 2, image, blue);
	line5(b * 3 / 2 - o / 2, c * 3 / 2 - o / 2, image, blue);
	line5(c * 3 / 2 - o / 2, d * 3 / 2 - o / 2, image, blue);
	line5(d * 3 / 2 - o / 2, e * 3 / 2 - o / 2, image, blue);
	line5(e * 3 / 2 - o / 2, a * 3 / 2 - o / 2, image, blue);

	//需要修正，题目给的是按照中心O
	//shear
	line5(shearing(1.0f / 3.0f)*a, shearing(1.0f / 3.0f)*b, image, yellow);
	line5(shearing(1.0f / 3.0f)*b, shearing(1.0f / 3.0f)*c, image, yellow);
	line5(shearing(1.0f / 3.0f)*c, shearing(1.0f / 3.0f)*d, image, yellow);
	line5(shearing(1.0f / 3.0f)*d, shearing(1.0f / 3.0f)*e, image, yellow);
	line5(shearing(1.0f / 3.0f)*e, shearing(1.0f / 3.0f)*a, image, yellow);

	//rotate
	line5(rotate(PI / 4.0f)*a, rotate(PI / 4.0f)* b, image, red);
	line5(rotate(PI / 4.0f)*b, rotate(PI / 4.0f)*c, image, red);
	line5(rotate(PI / 4.0f)*c, rotate(PI / 4.0f)*d, image, red);
	line5(rotate(PI / 4.0f)*d, rotate(PI / 4.0f)*e, image, red);
	line5(rotate(PI / 4.0f)*e, rotate(PI / 4.0f)*a, image, red);

	image.flip_vertically();
	image.write_tga_file("Lesson4/scale.tga");
}

//from Lesson3_main
//为了保持较好的阅读性，没有单独出去
Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
	auto res = cross(Vec3f(B.x - A.x, C.x - A.x, A.x - P.x), Vec3f(B.y - A.y, C.y - A.y, A.y - P.y));
	if (std::abs(res.z) < 0.001f)
		return { -1,1,1 };
	return { 1.0f - (res.x + res.y) / res.z, res.x / res.z,res.y / res.z };
}
//注意重心坐标的返回顺序，要严格按照res计算顺序

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

//仅仅适用与摄像机在z轴上
inline Matrix projection(float cameraPos) {
	Matrix tmp = Matrix::identity();
	tmp[3][2] = -1.0f / cameraPos;

	return tmp;
}

inline Matrix viewport(int x, int y, int w, int h) {
	Matrix tmp = Matrix::identity();
	tmp[0][0] = w / 2.f;
	tmp[1][1] = h / 2.f;
	tmp[2][2] = depth / 2.f;//暂时没用

	tmp[0][3] = x+w / 2.f;
	tmp[1][3] = y+h / 2.f;
	tmp[2][3] = depth / 2.f;

	return tmp;
}

Vec3f vec4Tovec3(Vec4f m) {
	return Vec3f(int(m[0] / m[3]), int(m[1] / m[3]),int(m[2] / m[3]));
}

void test4_2() {
	std::ofstream f("Lesson4/input1.txt");
	TGAImage image(width, height, TGAImage::RGB);
	model = new Model("obj/african_head/african_head.obj");
	float* zbuffer = new float[width*height];
	//注意当分配内存过大时，栈会溢出。。。 一定要用堆

	//这样写虽然简洁但是  一定记得加分号。。。。。。。。。。。
	for (int i = width * height; i--; zbuffer[i] = -FLT_MAX);

	auto Viewport = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	auto Projection = projection(cameraPos.z);

	Vec3f light_dir(0, 0, 1);

	for (int i = 0; i < model->nfaces(); ++i) {
		Vec3f screen_coord[3];
		Vec3f tmp[3];
		Vec2f UV[3];
		for (int j = 0; j < 3; ++j) {
			tmp[j] = model->vert(i, j);
			UV[j] = model->uv(i, j);	
			screen_coord[j] = vec4Tovec3(Viewport * Projection *
				Vec4f(tmp[j].x , tmp[j].y , tmp[j].z, 1.0f));
			f << screen_coord[j];
			//经过数据对比，我发现精度太高不是件好事。。。
		}
		f << std::endl;
		triangleUV(screen_coord[0], screen_coord[1], screen_coord[2], UV, image, zbuffer);
	}

	image.flip_vertically();
	image.write_tga_file("Lesson4/africanUV.tga");

	// dump z-buffer (debugging purposes only)
	TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			zbimage.set(i, j, TGAColor(zbuffer[i + j * width]));
		}
	}
	zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	zbimage.write_tga_file("Lesson4/zbuffer.tga");
	
	delete[]zbuffer;
	delete model;
	f.close();
}

int main(){
	test4_2();
}	