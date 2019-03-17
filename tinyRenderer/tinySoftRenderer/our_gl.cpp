/*
*
*
*@author: Aver Jing
*@description：
*@date：
*
*
*/

#include "our_gl.h"


//Matrix矩阵还没有移动构造，以后优化
//暂时考虑使用全局的
Matrix Projection = Matrix::identity();
Matrix Projection2 = Matrix::identity();
Matrix Projection3 = Matrix::identity();
Matrix OrthoProjection = Matrix::identity();
Matrix LookAt = Matrix::identity();
Matrix ViewPort = Matrix::identity();
Matrix Translate = Matrix::identity();

//仅仅适用与摄像机在z轴上
void projection(float cameraPos) {
	Projection[3][2] = -1.0f/cameraPos;
}

void projection4(float offset) {
	Projection[3][2] = offset;
}

void projection2(float r, float t, float zNear, float zFar) {
	Projection2[0][0] = zNear / r;
	Projection2[1][1] = zNear / t;
	Projection2[2][2] = -(zNear + zFar) / (zFar - zNear);
	Projection2[2][3] = -2.0f*zFar*zNear / (zFar - zNear);
	Projection2[3][2] = -1;
}

void projection3(float fovy, float aspect, float zNear, float zFar) {
	float value = tan(fovy / 2.0f);
	Projection3[0][0] = 1.0f / (value * aspect);
	Projection3[1][1] = 1.0f / (value);
	Projection3[2][2] = -(zNear + zFar) / (zFar - zNear);
	Projection3[2][3] = -2.0f * zFar*zNear / (zFar - zNear);
	Projection3[3][2] = -1;
}

void ortho(float l, float r, float b, float t, float zNear, float zFar) {
	OrthoProjection[0][0] = 1.0f / r;
	OrthoProjection[1][1] = 1.0f / t;
	OrthoProjection[2][2] = -2 / (zFar - zNear);
	OrthoProjection[2][3] = -(zFar + zNear) / (zFar - zNear);
}

void viewport(int x, int y, int w, int h) {
	ViewPort[0][0] = w / 2.f;
	ViewPort[1][1] = h / 2.f;
	ViewPort[2][2] = 255.0f / 2.f;//暂时没用

	ViewPort[0][3] = x + w / 2.f;
	ViewPort[1][3] = y + h / 2.f;
	ViewPort[2][3] = 255.0f / 2.f;
}

void lookat(Vec3f eye, Vec3f center, Vec3f up)
{
	Vec3f z = (eye - center).normalize();
	Vec3f x = cross(up, z).normalize();
	Vec3f y = cross(z, x).normalize();
	for (int i = 0; i < 3; ++i) {
		LookAt[0][i] = x[i];
		LookAt[1][i] = y[i];
		LookAt[2][i] = z[i];
		//LookAt[i][3] = -eye[i];
		//问题出在lookat矩阵。
		//上边地是正确地

		//LookAt[i][3] = -center[i];
		//可是在Lesson6  这种写法效果没问题，另外两种出错。
		//我觉得应该是摄像机坐标系圆心坐标有关
		//LookAt[i][3] = -center[i]; 圆心坐标应该在center
		//LookAt[i][3] = -eye[i];  应该在eye
		//没有验证，但是物体大小又差，应该是距离远近的关系

		
	}
	//glm实现地是这一种
	LookAt[0][3] = -(eye * x);
	LookAt[1][3] = -(eye * y);
	LookAt[2][3] = -(eye * z);
	
	//注意当验证作者教程时，要使用
	//LookAt[0][3] = -(center * x);
	//LookAt[1][3] = -(center * y);
	//LookAt[2][3] = -(center * z);
}

Matrix rotateByZaxis(float theta)
{
	Matrix tmp = Matrix::identity();
	tmp[0][0] = cos(theta);
	tmp[0][1] = -sin(theta);
	tmp[1][0] = sin(theta);
	tmp[1][1] = cos(theta);
	return tmp;
}

Matrix rotateByYaxis(float theta)
{
	Matrix tmp = Matrix::identity();
	tmp[0][0] = cos(theta);
	tmp[0][2] = sin(theta);
	tmp[2][0] = -sin(theta);
	tmp[2][2] = cos(theta);
	return tmp;
}

Matrix translate(Vec3f v)
{
	Translate[0][3] = v.x;
	Translate[1][3] = v.y;
	Translate[2][3] = v.z;
	return Translate;
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

