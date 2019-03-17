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
#pragma once
#include <algorithm>
#include "../geometry.h"
#include "tgaimage.h"

//extern const int depth;

void viewport(int x, int y, int w, int h);
void projection(float coeff = 0.0f); //coeff = -1/c
void projection4(float coeff = 0.0f);
void lookat(Vec3f eye, Vec3f center, Vec3f up);
void projection2(float r, float t, float zNear, float zFar);
void projection3(float fovy, float aspect, float zNear, float zFar);
void ortho(float l, float r, float b, float t, float zNear, float zFar);
Matrix translate(Vec3f v);
Matrix rotateByZaxis(float theta);
Matrix rotateByYaxis(float theta);


Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P);
//void triangle(Vec3f A, Vec3f B, Vec3f C, TGAImage& image, float zbuffer[])