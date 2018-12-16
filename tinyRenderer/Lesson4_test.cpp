/*
*
*
*@author: Aver Jing
*@description：
*@date：
*
*
*/
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const int width = 800;
const int height = 800;
const int depth = 255;

Model *model = NULL;
float *zbuffer = NULL;
Vec3f light_dir(0, 0, -1);
Vec3f camera(0, 0, 3);

Vec3f m2v(Matrix m) {
	return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

Matrix v2m(Vec3f v) {
	Matrix m=Matrix::identity();
	m[0][0] = v.x;
	m[1][0] = v.y;
	m[2][0] = v.z;
	m[3][0] = 1.f;
	return m;
}

Matrix viewport(int x, int y, int w, int h) {
	Matrix m = Matrix::identity();
	m[0][3] = x + w / 2.f;
	m[1][3] = y + h / 2.f;
	m[2][3] = depth / 2.f;

	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	m[2][2] = depth / 2.f;
	return m;
}

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
	auto res = cross(Vec3f(B.x - A.x, C.x - A.x, A.x - P.x), Vec3f(B.y - A.y, C.y - A.y, A.y - P.y));
	if (std::abs(res.z) < 0.001f)
		return { -1,1,1 };
	return { 1.0f - (res.x + res.y) / res.z, res.x / res.z,res.y / res.z };
}

//for uv
void triangle(Vec3f A, Vec3f B, Vec3f C, Vec2f* uv, TGAImage& image, float zbuffer[]) {
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

int main() {
	model = new Model("obj/african_head/african_head.obj");
	std::ofstream f("Lesson4/input2.txt");

	zbuffer = new float[width*height];
	for (int i = 0; i < width*height; i++) {
		zbuffer[i] = std::numeric_limits<int>::min();
	}

	{ // draw the model
		Matrix Projection = Matrix::identity();
		Matrix ViewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		Projection[3][2] = -1.f / camera.z;

		TGAImage image(width, height, TGAImage::RGB);
		for (int i = 0; i < model->nfaces(); i++) {
			std::vector<int> face = model->face(i,0);
			Vec3i screen_coords[3];
			Vec3f world_coords[3];
			for (int j = 0; j < 3; j++) {
				Vec3f v = model->vert(face[j]);
				screen_coords[j] = m2v(ViewPort*Projection*v2m(v));
				world_coords[j] = v;
				f << screen_coords[j];
			}
			f << std::endl;
			Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
			n.normalize();
			float intensity = n * light_dir;
			if (intensity > 0) {
				Vec2f uv[3];
				for (int k = 0; k < 3; k++) {
					uv[k] = model->uv(i, k);
				}
				triangle(screen_coords[0], screen_coords[1], screen_coords[2], uv, image, zbuffer);
			}
		}

		image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		image.write_tga_file("Lesson4/output.tga");
	}

	{ // dump z-buffer (debugging purposes only)
		TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				zbimage.set(i, j, TGAColor(zbuffer[i + j * width]));
			}
		}
		zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		zbimage.write_tga_file("Lesson4/zbuffer2.tga");
	}
	delete model;
	delete[] zbuffer;
	return 0;
}