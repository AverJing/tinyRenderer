#include "our_gl_old.h"

Matrix ModelView = Matrix::identity();
Matrix ViewPort = Matrix::identity();
Matrix Projection = Matrix::identity();

void viewport(int x, int y, int w, int h) {
	ViewPort[0][0] = w / 2.0f;
	ViewPort[1][1] = h / 2.0f;
	ViewPort[2][2] = 255.0f / 2.0f;  //depth是近平面和远平面的距离
	ViewPort[0][3] = x + w / 2.0f;
	ViewPort[1][3] = y + h / 2.0f;
	ViewPort[2][3] = 255.0f / 2.0f;
}

void projection(float coeff)
{
	Projection[3][2] = coeff;
}

void lookat(Vec3f eye, Vec3f center, Vec3f up)
{
	Vec3f z = (eye - center).normalize();
	Vec3f x = cross(up, z).normalize();
	Vec3f y = cross(z, x).normalize();
	for (int i = 0; i < 3; ++i) {
		ModelView[0][i] = x[i];
		ModelView[1][i] = y[i];
		ModelView[2][i] = z[i];
		ModelView[i][3] = -center[i];
	}
	//ModelView[0][3] = -(center * x);
	//ModelView[1][3] = -(center * y);
	//ModelView[2][3] = -(center * z);

}


Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P) {
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

void triangle(Vec4f * pts, IShader & shader, TGAImage & image, TGAImage & zbuffer)
{
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::min(bboxmin[j], pts[i][j]/ pts[i][3]);
			bboxmax[j] = std::max(bboxmax[j], pts[i][j]/ pts[i][3]);
		}
	}
	Vec2i P;  //如果类型变为Vec2f， 那么就会有黑色的线
	//奇怪
	//还是和之前的问题一样。  viewport后的坐标就是整型

	TGAColor color;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; ++P.x) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; ++P.y) {
			Vec3f bc = barycentric(proj<2>(pts[0]/pts[0][3]), proj<2>(pts[1] / pts[1][3]), proj<2>(pts[2] / pts[2][3]), P);
			float z = pts[0][2] * bc.x + pts[1][2] * bc.y + pts[2][2] * bc.z;
			float w = pts[0][3] * bc.x + pts[1][3] * bc.y + pts[2][3] * bc.z;
			int frag_depth = std::max(0, std::min(255, int(z/w + 0.5)));
			// z/w?

			if (bc.x < 0 || bc.y < 0 || bc.z<0 || zbuffer.get(P.x, P.y)[0]>frag_depth) continue;
			bool discard = shader.fragment(bc, color);
			if (!discard) {
				zbuffer.set(P.x, P.y, TGAColor(frag_depth));
				image.set(P.x, P.y, color);
			}
		}
	}
}

/*
Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P) {
	Vec3f s[2];
	for (int i = 2; i--; ) {
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}
	Vec3f u = cross(s[0], s[1]);
	if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void triangle(Vec4f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer) {
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
			bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
		}
	}
	Vec2i P;
	TGAColor color;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f c = barycentric(proj<2>(pts[0] / pts[0][3]), proj<2>(pts[1] / pts[1][3]), proj<2>(pts[2] / pts[2][3]), proj<2>(P));
			float z = pts[0][2] * c.x + pts[1][2] * c.y + pts[2][2] * c.z;
			float w = pts[0][3] * c.x + pts[1][3] * c.y + pts[2][3] * c.z;
			int frag_depth = std::max(0, std::min(255, int(z / w + .5)));
			if (c.x < 0 || c.y < 0 || c.z<0 || zbuffer.get(P.x, P.y)[0]>frag_depth) continue;
			bool discard = shader.fragment(c, color);
			if (!discard) {
				zbuffer.set(P.x, P.y, TGAColor(frag_depth));
				image.set(P.x, P.y, color);
			}
		}
	}
}*/

void triangle(Vec4f *pts, IShader &shader, TGAImage &image, float *zbuffer) {
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
			bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
		}
	}
	Vec2i P;
	TGAColor color;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f c = barycentric(proj<2>(pts[0] / pts[0][3]), proj<2>(pts[1] / pts[1][3]), proj<2>(pts[2] / pts[2][3]), proj<2>(P));
			float z = pts[0][2] * c.x + pts[1][2] * c.y + pts[2][2] * c.z;
			float w = pts[0][3] * c.x + pts[1][3] * c.y + pts[2][3] * c.z;
			int frag_depth = z / w;
			if (c.x < 0 || c.y < 0 || c.z<0 || zbuffer[P.x + P.y*image.get_width()]>frag_depth) continue;
			bool discard = shader.fragment(c, color);
			if (!discard) {
				zbuffer[P.x + P.y*image.get_width()] = frag_depth;
				image.set(P.x, P.y, color);
			}
		}
	}
}

//void triangle(mat<4, 3, float> &clipc, IShader &shader, TGAImage &image, float *zbuffer) {
//	mat<3, 4, float> pts = (ViewPort*clipc).transpose(); // transposed to ease access to each of the points
//	mat<3, 2, float> pts2;
//	for (int i = 0; i < 3; i++) pts2[i] = proj<2>(pts[i] / pts[i][3]);
//
//	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
//	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
//	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
//	for (int i = 0; i < 3; i++) {
//		for (int j = 0; j < 2; j++) {
//			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts2[i][j]));
//			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts2[i][j]));
//		}
//	}
//	Vec2i P;
//	TGAColor color;
//	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
//		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
//			Vec3f bc_screen = barycentric(pts2[0], pts2[1], pts2[2], P);
//			//Vec3f bc_clip = Vec3f(bc_screen.x / pts[0][3], bc_screen.y / pts[1][3], bc_screen.z / pts[2][3]);
//			//bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);
//			//float frag_depth = clipc[2] * bc_clip;
//			//为什么这么计算
//
//			float z = pts[0][2] * bc_screen.x + pts[1][2] * bc_screen.y + pts[2][2] * bc_screen.z;
//			float w = pts[0][3] * bc_screen.x + pts[1][3] * bc_screen.y + pts[2][3] * bc_screen.z;
//			float frag_depth = z / w;
//			
//			
//			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z<0 || zbuffer[P.x + P.y*image.get_width()]>frag_depth) continue;
//			bool discard = shader.fragment(Vec3f(P.x, P.y, frag_depth), bc_screen, color);
//			if (!discard) {
//				zbuffer[P.x + P.y*image.get_width()] = frag_depth;
//				image.set(P.x, P.y, color);
//			}
//		}
//	}
//}
