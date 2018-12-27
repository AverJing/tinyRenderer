#include "Rasterizer.h"
#include <algorithm>

void Rasterizer::DrawPixel(int x, int y, const Color &color)
{
	if (x<0 || y<0 || x>=context->width || y>=context->height) return;
	context->backBuffer[x + y * context->width] = (color.GetA() << 24) | (color.GetR() << 16) | (color.GetG() << 8) | (color.GetB() << 0);
}

void Rasterizer::DrawOneline(Vec2i p0, Vec2i p1, const Color &color)
{
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
			DrawPixel(y, x, color);
		}
		else {
			DrawPixel(x, y, color);
		}
	}
}

Vec3f Rasterizer::barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P)
{
	auto res = cross(Vec3f(B.x - A.x, C.x - A.x, A.x - P.x), Vec3f(B.y - A.y, C.y - A.y, A.y - P.y));
	if (std::abs(res.z) < 0.001f)
		return { -1,1,1 };
	return { 1.0f - (res.x + res.y) / res.z, res.x / res.z,res.y / res.z };

}
//注意重心坐标的返回顺序，要严格按照res计算顺序

void Rasterizer::triangle(Vec3f A, Vec3f B, Vec3f C, const Color & color)
{
	Vec2f bboxmin(std::max(0.0f, std::min({ A.x,B.x,C.x })), std::max(0.0f, std::min({ A.y,B.y,C.y })));
	//和0比较是为了防止超出屏幕
	Vec2f bboxmax(std::min(static_cast<float>(context->width - 1), std::max({ A.x,B.x,C.x })), std::min(static_cast<float>(context->height - 1), std::max({ A.y,B.y,C.y })));

	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; ++P.x) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; ++P.y) {
			auto bc = barycentric(A, B, C, P);
			if (bc.x < 0 || bc.y < 0 || bc.z < 0)continue;
			P.z = bc.x*A.z + bc.y*B.z + bc.z*C.z;
			DrawPixel(P.x, P.y, color);
			/*暂时不考虑深度
			if (context->depthBuffer[int(P.x + P.y*context->width)] < P.z) {//int(P.x) + int(P.y)*width
				context->depthBuffer[int(P.x + P.y*context->width)] = P.z;
				DrawPixel(P.x, P.y, color);
			}*/
		}
	}
}

//for uv
void Rasterizer::triangleUV(Vec3f A, Vec3f B, Vec3f C, Vec2f* uv, Model* model) {
	Vec2f bboxmin(std::max(0.0f, std::min({ A.x,B.x,C.x })), std::max(0.0f, std::min({ A.y,B.y,C.y })));
	//和0比较是为了防止超出屏幕
	Vec2f bboxmax(std::min(static_cast<float>(context->width - 1), std::max({ A.x,B.x,C.x })), std::min(static_cast<float>(context->height - 1), std::max({ A.y,B.y,C.y })));

	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; ++P.x) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; ++P.y) {
			auto bc = barycentric(A, B, C, P);
			if (bc.x < 0 || bc.y < 0 || bc.z < 0)continue;
			P.z = bc.x*A.z + bc.y*B.z + bc.z*C.z;
			auto UV = uv[0] * bc.x + uv[1] * bc.y + uv[2] * bc.z;
			if (context->depthBuffer[int(P.x + P.y*context->width)] < P.z) {//int(P.x) + int(P.y)*width
				context->depthBuffer[int(P.x + P.y*context->width)] = P.z;
				DrawPixel(P.x, P.y, model->diffuse(UV));
			}
		}
	}
}

bool Rasterizer::flip_vertically() {
	if (!context) return false;
	unsigned long bytes_per_line = context->width * context->bpp;
	unsigned char *line = new unsigned char[bytes_per_line];
	int half = context->height >> 1;
	for (int j = 0; j < half; j++) {
		unsigned long l1 = j * bytes_per_line;
		unsigned long l2 = (context->height - 1 - j)*bytes_per_line;
		//将(data+l1)和(data+l2)中的数据交换
		//在opengl中y轴图片的y轴相反
		//注意指针+1，移动的位置和其本事类型有关
		memmove(line, (char *)(context->backBuffer)+ l1, bytes_per_line);
		memmove((char *)(context->backBuffer) + l1, (char *)(context->backBuffer) + l2, bytes_per_line);
		memmove((char *)(context->backBuffer) + l2, line, bytes_per_line);
	}
	delete[] line;
	return true;
}


