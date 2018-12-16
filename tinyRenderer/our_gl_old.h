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
#include "geometry.h"
#include "tgaimage.h"

//extern const int depth;

void viewport(int x, int y, int w, int h);
void projection(float coeff = 0.0f); //coeff = -1/c
void lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader {
	virtual ~IShader() {};
	virtual Vec4f vertex(int iface, int nthervt) = 0;
	virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
	
	//lesson 8
	//virtual bool fragment(Vec3f gl_FragCoord, Vec3f bar, TGAColor &color) = 0;
};

void triangle(Vec4f* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer);
void triangle(Vec4f *pts, IShader &shader, TGAImage &image, float *zbuffer);
void triangle(mat<4, 3, float> &clipc, IShader &shader, TGAImage &image, float *zbuffer);
