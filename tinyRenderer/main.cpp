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

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
std::shared_ptr<Model> model = nullptr;
const int width = 800;
const int height = 700;

//TGAImage depth(width, height, TGAImage::RGB);
//TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
float *shadowbuffer = nullptr;


//C++ primer  CH12.2
//std::unique_ptr<float[]> zbuffer(new float[width*height]);
Vec3f light_dir(1, 1, 0);
Vec3f eye(1, 1, 4);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);
Vec3f camera(0, 0, 3);

extern Matrix ModelView;
extern Matrix ViewPort;
extern Matrix Projection;

struct GouraudShader :public IShader {
	Vec3f varying_intensity;

	virtual Vec4f vertex(int iface, int nthvert) override {
		varying_intensity[nthvert] = std::max(0.0f, model->normal(iface, nthvert)*light_dir);
		Vec4f gl_vertex = embed<4>(model->vert(iface, nthvert));
		return ViewPort * Projection * ModelView * gl_vertex;
	}

	/*
	virtual bool fragment(Vec3f bar, TGAColor& color) override {
		auto intensity = varying_intensity * bar;
		color = TGAColor(255,255,255) * intensity;
		return false;
	}
	*/

	virtual bool fragment(Vec3f bar, TGAColor& color) override {
		auto intensity = varying_intensity * bar;
		if (intensity > .85f) intensity = 1.0f;
		else if(intensity > .60f)intensity = 0.8f;
		else if(intensity > .45f)intensity = 0.6f;
		else if (intensity > .30f)intensity = 0.45f;
		else if (intensity > .15f)intensity = 0.30f;
		else intensity = 0.0f;

		color = TGAColor(255, 255, 255) * intensity;
		return false;
	}
};

struct shader : public IShader
{
	//version 1.0
	/*
	Vec3f varying_intensity;
	mat<2, 3, float> varying_uv;

	virtual Vec4f vertex(int iface, int nthvert) override {
		varying_intensity[nthvert] = std::max(0.0f, model->normal(iface, nthvert)*light_dir);
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
		return ViewPort * Projection*ModelView*gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color) {
		auto intensity = varying_intensity * bar;
		auto uv = varying_uv * bar;
		color = model->diffuse(uv)*intensity;
		return false;
	}
	*/

	//version 3.0
	/*
	mat<2, 3, float> varying_uv;
	mat<4, 4, float> uniform_M;//Projection*ModelView
	mat<4, 4, float> uniform_MIT;//(Projection*ModelView).invert_transpose

	virtual Vec4f vertex(int iface, int nthvert) override {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
		return ViewPort * Projection*ModelView*gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color) {
		auto uv = varying_uv * bar;
		auto n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();
		auto l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();
		float intensity = std::max(0.0f, n*l);
		color = model->diffuse(uv)*intensity;
		return false;
	}*/

	//效果不如3.0  为什么？
	//version 2.0
	/*
	mat<2, 3, float> varying_uv;
	virtual Vec4f vertex(int iface, int nthvert) override {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
		return ViewPort * Projection*ModelView*gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color) {
		auto uv = varying_uv * bar;
		float intensity = std::max(0.0f, model->normal(uv)*light_dir);
		color = model->diffuse(uv)*intensity;
		return false;
	}*/

	//version 4.0
	/*
	mat<2, 3, float> varying_uv;
	mat<4, 4, float> uniform_M;//Projection*ModelView
	mat<4, 4, float> uniform_MIT;//(Projection*ModelView).invert_transpose

	virtual Vec4f vertex(int iface, int nthvert) override {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
		return ViewPort * Projection*ModelView*gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color) {
		auto uv = varying_uv * bar;
		auto n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();
		auto l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();
		auto r = (n*(n*l*2.0f) - l).normalize();
		float spec = pow(std::max(0.0f, r.z), model->specular(uv));
		//此处并没有  观察向量
		float diff = std::max(0.0f, n*l);
		color = model->diffuse(uv);
		for (int i = 0; i < 3; ++i)
			color[i] = std::min<float>(5 + color[i] * (diff + 0.6*spec), 255);
		return false;
	}
	*/

	//version 5.0
	/*
	mat<2, 3, float> varying_uv;
	mat<4, 4, float> uniform_M;//Projection*ModelView
	mat<4, 4, float> uniform_MIT;//(Projection*ModelView).invert_transpose
	mat<3, 3, float> varying_nrm;//normal per vertex to be interpolated by FS
	mat<4, 4, float> varying_tri;// triangle coordinates (clip coordinates), written by VS, read by FS
	mat<3, 3, float> ndc_tri;//triangle in normalized device coordinates

	virtual Vec4f vertex(int iface, int nthvert) override {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		varying_nrm.set_col(nthvert, proj<3>(uniform_MIT * embed<4>(model->normal(iface, nthvert))));
		Vec4f gl_Vertex = ViewPort * Projection * ModelView* embed<4>(model->vert(iface, nthvert));
		varying_tri.set_col(nthvert, gl_Vertex);
		ndc_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color) {
		auto uv = varying_uv * bar;
		auto bn = varying_nrm * bar;
		//float diff = std::max(0.0f, bn*light_dir);

		mat<3, 3, float> A;
		A[0] = ndc_tri.col(1) - ndc_tri.col(0);
		A[1] = ndc_tri.col(2) - ndc_tri.col(0);
		A[2] = bn;
		auto AI = A.invert();

		auto i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
		auto j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);
		
		mat<3, 3, float> B;
		B.set_col(0, i.normalize());
		B.set_col(1, j.normalize());
		B.set_col(2, bn);

		auto n = (B*model->normal(uv)).normalize();
		//perturbed normal from the texture and apply the basis change 
		//from the tangent basis to the global coordinates.

		float diff = std::max(0.0f, n*light_dir);
		color = model->diffuse(uv) * diff;
		
		return false;
	}*/

	//version 6.0

	mat<4, 4, float> uniform_M;   //  Projection*ModelView
	mat<4, 4, float> uniform_MIT; // (Projection*ModelView).invert_transpose()
	mat<4, 4, float> uniform_Mshadow; // transform framebuffer screen coordinates to shadowbuffer screen coordinates
	mat<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	mat<3, 3, float> varying_tri; // triangle coordinates before Viewport transform, written by VS, read by FS

	shader(Matrix M, Matrix MIT, Matrix MS) : uniform_M(M), uniform_MIT(MIT), uniform_Mshadow(MS), varying_uv(), varying_tri() {}

	virtual Vec4f vertex(int iface, int nthvert) {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		Vec4f gl_Vertex = ViewPort * Projection*ModelView*embed<4>(model->vert(iface, nthvert));
		varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor &color) {
		Vec4f sb_p = uniform_Mshadow * embed<4>(varying_tri*bar); // corresponding point in the shadow buffer
		sb_p = sb_p / sb_p[3];
		int idx = int(sb_p[0]) + int(sb_p[1])*width; // index in the shadowbuffer array
		float shadow = .3 + .7*(shadowbuffer[idx] < sb_p[2]); // magic coeff to avoid z-fighting
		Vec2f uv = varying_uv * bar;                 // interpolate uv for the current pixel
		Vec3f n = proj<3>(uniform_MIT*embed<4>(model->normal(uv))).normalize(); // normal
		Vec3f l = proj<3>(uniform_M  *embed<4>(light_dir)).normalize(); // light vector
		Vec3f r = (n*(n*l*2.f) - l).normalize();   // reflected light
		float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
		float diff = std::max(0.f, n*l);
		TGAColor c = model->diffuse(uv);
		for (int i = 0; i < 3; i++) color[i] = std::min<float>(c[i] * shadow *(0.6 + 1.2*diff + .6*spec), 255);
		return false;
	}
};

struct DepthShader :public IShader {
	mat<3, 3, float> varying_tri;

	virtual Vec4f vertex(int iface, int nthvert) override {
		Vec4f gl_Vertex = ViewPort * Projection * ModelView* embed<4>(model->vert(iface, nthvert));
		varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color) {
		auto p = varying_tri * bar;
		color = TGAColor(255, 255, 255, 255)*(p.z / 255);

		return false;
	}
};

/*
void rasterize(Vec2i p0, Vec2i p1, TGAImage& image, const TGAColor& color, int ybuffer[]) {
	if (p0.x > p1.x)
		std::swap(p0, p1);

	for (int x = p0.x; x <= p1.x; ++x) {
		float t = (x - p0.x) / (float)(p1.x - p0.x);
		int y = p0.y*(1 - t) + p1.y*t;

		if (ybuffer[x] < y) {
			ybuffer[x] = y;
			image.set(x, 0, color);
			image.set(x, 1, color);
			image.set(x, 2, color);
		}
	}
}

Vec3f m2v(const Matrix& m) {
	return Vec3f((int)(m[0][0] / m[3][0]), (int)(m[1][0] / m[3][0]), (int)(m[2][0] / m[3][0]));
}

Matrix v2m(const Vec3f& v) {
	Matrix m; //default
	m[0][0] = v.x;
	m[1][0] = v.y;
	m[2][0] = v.z;
	m[3][0] = 1.0f;
	return m;
}

Vec3f world2screen(Vec3f v) {
	return Vec3f(int((v.x + 1.)*width / 2. + .5), int((v.y + 1.)*height / 2. + .5), v.z);
}*/

int main(){
	/*
	lookat(eye, center, up);
	viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	projection(-1.0f / (eye - center).norm());
	light_dir.normalize();

	TGAImage image(width, height, TGAImage::RGB);
	TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

	model = std::make_shared<Model>("obj/african_head/african_head.obj");

	//GouraudShader shader;
	shader s;
	s.uniform_M = Projection * ModelView;
	s.uniform_MIT = (s.uniform_M).invert_transpose();
	for (int i = 0; i < model->nfaces(); ++i) {
		Vec4f screen_coords[3];
		for (int j = 0; j < 3; ++j) {
			screen_coords[j] = s.vertex(i, j);
		}
		triangle(screen_coords, s, image, zbuffer);
	}
	
	image.flip_vertically();
	zbuffer.flip_vertically();

	image.write_tga_file("output.tga");
	zbuffer.write_tga_file("zbuffer.tga");*/
	
	float *zbuffer = new float[width*height];
	shadowbuffer = new float[width*height];
	for (int i = width * height; --i; ) {
		zbuffer[i] = shadowbuffer[i] = -std::numeric_limits<float>::max();
	}

	//lookat(light_dir, up, center);
	//viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	//projection(0);
	light_dir.normalize();
	model = std::make_shared<Model>("obj/diablo3_pose/diablo3_pose.obj");

	//GouraudShader shader;
	/*
	DepthShader depthshader;
	for (int i = 0; i < model->nfaces(); ++i) {
		Vec4f screen_coords[3];
		for (int j = 0; j < 3; ++j) {
			screen_coords[j] = depthshader.vertex(i, j);
		}
		triangle(screen_coords, depthshader, depth, zbuffer);
	}

	depth.flip_vertically();
	zbuffer.flip_vertically();

	depth.write_tga_file("output.tga");
	zbuffer.write_tga_file("zbuffer.tga");

	Matrix M = ViewPort * Projection*ModelView;

	{ // rendering the frame buffer
		TGAImage frame(width, height, TGAImage::RGB);
		lookat(eye, center, up);
		viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		projection(-1.f / (eye - center).norm());

		shader shader(ModelView, (Projection*ModelView).invert_transpose(), M*(ViewPort*Projection*ModelView).invert());
		Vec4f screen_coords[3];
		for (int i = 0; i < model->nfaces(); i++) {
			for (int j = 0; j < 3; j++) {
				screen_coords[j] = shader.vertex(i, j);
			}
			triangle(screen_coords, shader, frame, zbuffer);
		}
		frame.flip_vertically(); // to place the origin in the bottom left corner of the image
		frame.write_tga_file("framebuffer.tga");
	}*/

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

	Matrix M = ViewPort * Projection*ModelView;

	{ // rendering the frame buffer
		TGAImage frame(width, height, TGAImage::RGB);
		lookat(eye, center, up);
		viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		projection(-1.f / (eye - center).norm());

		shader shader(ModelView, (Projection*ModelView).invert_transpose(), M*(ViewPort*Projection*ModelView).invert());
		//shader shader;
		//shader.uniform_M = Projection * ModelView;
		//shader.uniform_MIT = (shader.uniform_M).invert_transpose();
		Vec4f screen_coords[3];
		for (int i = 0; i < model->nfaces(); i++) {
			for (int j = 0; j < 3; j++) {
				screen_coords[j] = shader.vertex(i, j);
			}
			triangle(screen_coords, shader, frame, zbuffer);
		}
		frame.flip_vertically(); // to place the origin in the bottom left corner of the image
		frame.write_tga_file("framebuffer.tga");
	}

	delete[] zbuffer;
	delete[] shadowbuffer;
	return 0;
	
}	