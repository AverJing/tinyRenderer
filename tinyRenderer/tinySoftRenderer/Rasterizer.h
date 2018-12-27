#pragma once

#include "RenderContex.h"
#include "Color.h"
#include "../geometry.h"
#include "model.h"

class Rasterizer {
public:
	Rasterizer(RenderContext* p) :context(p) {};
	~Rasterizer() {}

	void DrawPixel(int, int, const Color&);
	void DrawOneline(Vec2i , Vec2i, const Color&);

	Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P);
	void triangle(Vec3f A, Vec3f B, Vec3f C, const Color& color);
	void triangleUV(Vec3f A, Vec3f B, Vec3f C, Vec2f* uv, Model* model);

	bool flip_vertically();
private:
	RenderContext* context;
};