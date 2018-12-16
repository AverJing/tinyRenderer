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
#include <limits>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl_old.h"

Model *model = NULL;
float *shadowbuffer = NULL;

const int width = 800;
const int height = 800;

Vec3f light_dir(1, 1, 0);
Vec3f       eye(1, 1, 4);
Vec3f    center(0, 0, 0);
Vec3f        up(0, 1, 0);

extern Matrix ModelView;
extern Matrix ViewPort;
extern Matrix Projection;

struct DepthShader : public IShader {
	mat<3, 3, float> varying_tri;

	DepthShader() : varying_tri() {}

	virtual Vec4f vertex(int iface, int nthvert) override {
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
		gl_Vertex = ViewPort * Projection*ModelView*gl_Vertex;          // transform it to screen coordinates
		varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor &color) override{
		Vec3f p = varying_tri * bar;
		color = TGAColor(255, 255, 255)*(p.z / 255);
		return false;
	}
};

int main() {
	shadowbuffer = new float[width*height];
	for (int i = width * height; --i; ) {
		shadowbuffer[i] = -std::numeric_limits<float>::max();
	}

	model = new Model("obj/african_head/african_head.obj");
	light_dir.normalize();

	{ // rendering the shadow buffer
		TGAImage depth(width, height, TGAImage::RGB);
		lookat(light_dir, center, up);
		viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		projection(0);

		DepthShader depthshader;
		Vec4f screen_coords[3];
		for (int i = 0; i < model->nfaces(); i++) {
			for (int j = 0; j < 3; j++) {
				screen_coords[j] = depthshader.vertex(i, j);
			}
			triangle(screen_coords, depthshader, depth, shadowbuffer);
		}
		depth.flip_vertically(); // to place the origin in the bottom left corner of the image
		depth.write_tga_file("depth.tga");
	}

	delete model;
	delete[] shadowbuffer;
	return 0;
}