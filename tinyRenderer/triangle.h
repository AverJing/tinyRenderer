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
#include <algorithm>
#include "tgaimage.h"
#include "geometry.h"

extern const int width;
extern const int height;

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

//
Vec3f barycentric(Vec2i* pts, Vec2i p) {
	Vec3f u = cross(Vec3f(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - p[0]),
		Vec3f(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - p[1]));
	/* `pts` and `P` has integer value as coordinates
	   so `abs(u[2])` < 1 means `u[2]` is 0, that means
	   triangle is degenerate, in this case return something with negative coordinates */
	if (std::abs(u[2]) < 1) return Vec3f(-1, 1, 1);
	return Vec3f(1.0f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

Vec3f barycentric(Vec3f* pts, Vec3f p) {
	Vec3f u = cross(Vec3f(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - p[0]),
		Vec3f(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - p[1]));
	/*
	if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
	*/
	if (std::abs(u[2]) < 1) return Vec3f(-1, 1, 1);
	return Vec3f(1.0f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void triangle(Vec2i* pts, TGAImage& image, const TGAColor& color) {
	Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
	Vec2i bboxmax(0, 0);
	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 2; ++j) {
			bboxmin[j] = std::max(0, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	Vec2i p;
	for (p.x = bboxmin.x; p.x <= bboxmax.x; ++p.x) {
		for (p.y = bboxmin.y; p.y <= bboxmax.y; ++p.y) {
			auto bc_screen = barycentric(pts, p);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			image.set(p.x, p.y, color);
		}
	}
}
/*
void triangle(Vec3f* pts, float zbuffer[], TGAImage& image, const TGAColor& color) {
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 2; ++j) {
			bboxmin[j] = std::max(0.0f, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	Vec3f p;
	for (p.x = bboxmin.x; p.x <= bboxmax.x; ++p.x) {
		for (p.y = bboxmin.y; p.y <= bboxmax.y; ++p.y) {
			auto bc_screen = barycentric(pts, p);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			
			p.z = 0;
			for (int i = 0; i < 3; ++i)
				p.z += pts[i][2] * bc_screen[i];
			if (zbuffer[int(p.x + p.y*width)] < p.z) {
				zbuffer[int(p.x + p.y*width)] = p.z;
				image.set(p.x, p.y, color);
			}
			
		}
	}
}*/

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
	Vec3f s[2];
	for (int i = 2; i--; ) {
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}
	Vec3f u = cross(s[0], s[1]);
	if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	//注意推导过程中 原系数向量是  u v 1，故需要除以z分量
	return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void triangle(Vec3f *pts, std::unique_ptr<float[]>&zbuffer, TGAImage &image, TGAColor color) {
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			P.z = 0;
			for (int i = 0; i < 3; ++i) P.z += pts[i][2] * bc_screen[i];
			if (zbuffer[int(P.x + P.y*width)] < P.z) {
				zbuffer[int(P.x + P.y*width)] = P.z;
				image.set(P.x, P.y, color);
			}
		}
	}
}

void triangle(Vec3f *pts, std::unique_ptr<float[]>&zbuffer, TGAImage &image, TGAColor *color) {
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			P.z = 0;
			TGAColor interColor(0,0,0,255);
			for (int i = 0; i < 3; ++i) {
				P.z += pts[i][2] * bc_screen[i];
				interColor += color[i] * bc_screen[i];
			}

			if (zbuffer[int(P.x + P.y*width)] < P.z) {
				zbuffer[int(P.x + P.y*width)] = P.z;
				image.set(P.x, P.y, interColor);
			}
		}
	}
}

void triangle(Vec3f *pts, std::unique_ptr<float[]>&zbuffer, TGAImage &image, Vec2f *uv, std::shared_ptr<Model> model, float intensity) {
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			P.z = 0;
			Vec2f UV(0.0f, 0.0f);
			for (int i = 0; i < 3; ++i) {
				P.z += pts[i][2] * bc_screen[i];
				UV += uv[i] * bc_screen[i];
			}

			if (zbuffer[int(P.x + P.y*width)] < P.z) {
				zbuffer[int(P.x + P.y*width)] = P.z;
				image.set(P.x, P.y, model->diffuse(UV)*intensity);
			}
		}
	}
}
