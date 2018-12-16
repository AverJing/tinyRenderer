[Lesson 6](https://github.com/ssloy/tinyrenderer/wiki/Lesson-6:-Shaders-for-the-software-renderer)：软渲染器的着色器

### 回顾

- 回顾自己的源代码。
<img src="E:\GameLearning\tinyRenderer\Notes\CH06\01.png" style="zoom:60%" div align="center">


### 重构源代码
- main太过于臃肿，将main文件拆分两部分：
	- our_gl.cpp+h；将model，lookat，projection，viewport矩阵，画三角形算法放进去。
	- main.cpp 
	- 注意我仍然决定暂时，每一课独立写一个main，方便阅读。

- 下边就是our_gl.h的内容：
```C++
#include "tgaimage.h"
#include "geometry.h"

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;

void viewport(int x, int y, int w, int h);
void projection(float coeff=0.f); // coeff = -1/c
void lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader {
    virtual ~IShader();
    virtual Vec3i vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

void triangle(Vec4f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer);
```
- main.cpp代码如下：
```C++
#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

Model *model     = NULL;
const int width  = 800;
const int height = 800;

Vec3f light_dir(1,1,1);
Vec3f       eye(1,1,3);
Vec3f    center(0,0,0);
Vec3f        up(0,1,0);

struct GouraudShader : public IShader {
    Vec3f varying_intensity; // written by vertex shader, read by fragment shader

    virtual Vec4f vertex(int iface, int nthvert) {
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        return Viewport*Projection*ModelView*gl_Vertex; // transform it to screen coordinates
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
        color = TGAColor(255, 255, 255)*intensity; // well duh
        return false;                              // no, we do not discard this pixel
    }
};

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }

    lookat(eye, center, up);
    viewport(width/8, height/8, width*3/4, height*3/4);
    projection(-1.f/(eye-center).norm());
    light_dir.normalize();

    TGAImage image  (width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    GouraudShader shader;
    for (int i=0; i<model->nfaces(); i++) {
        Vec4f screen_coords[3];
        for (int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }

    image.  flip_vertically(); // to place the origin in the bottom left corner of the image
    zbuffer.flip_vertically();
    image.  write_tga_file("output.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    delete model;
    return 0;
}
```
- 它是如何工作的？跳过头文件，我们声明了一些全局变量：屏幕的宽度和高度，摄像机位置等。下一小结，我会解释GouraudShader结构，暂时跳过。main的实际功能是：
	- 解析obj文件
	- 初始化ModelView，Projection和Viewport矩阵（回忆如何生成这些矩阵的）。
	- 迭代模型中的所有三角形并光栅化。

- 最后一步也是最有趣的一步。外层循环迭代所有的三角形，内层循环迭代当前三角形的顶点并且调用顶点着色器。
	- 顶点着色器的主要目标是变换顶点坐标。次目标是给段着色器准备数据。

- 那之后发生了什么？我们执行光栅化管线过程。我们虽然不知道具体如何光栅化的（其实，在编程时我们应该准确知道。）
	- 顶点着色器的主要目标是决定每个像素的颜色。次目标是决定是否丢弃当前像素。

- OpenGL渲染管线如下：
<img src="E:\GameLearning\tinyRenderer\Notes\CH06\02.png" style="zoom:100%" div align="center">
- 由于时间限制，本教程仅仅使用顶点着色器和段着色器。但是在OpenGL中还允许使用其他着色器。
- 上边图蓝色的代表我们没有使用的，橘红色的是我们使用到的。事实上，我们的main函数就是图元处理过程。它调用顶点着色器。我们图元装载过程，因为我们直接渲染若干个三角形（和图元处理过程混合在一起）。三角形函数就是光栅化，对于在三角形中的点，它都会调用段着色器，之后会执行深度测试（z-buffer）。

### Gouraud着色器
<img src="E:\GameLearning\tinyRenderer\Notes\CH06\03.png" style="zoom:60%" div align="center"> 
- 我们看一下Gouraud着色器的代码：
```C++
Vec3f varying_intensity; // written by vertex shader, read by fragment shader
    virtual Vec4f vertex(int iface, int nthvert) {
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        return Viewport*Projection*ModelView*gl_Vertex; // transform it to screen coordinates
    }
```
-  varying是GLSL中的预留关键字，我使用varying_intensity是为了展示即将在Lesson9中讨论的GLSL。In varying variables we store data to be interpolated inside the triangle, and the fragment shaders get the interpolated value (for the current pixel).
-  在看下段着色器的代码
```C++
  Vec3f varying_intensity; // written by vertex shader, read by fragment shader
// [...]
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
        color = TGAColor(255, 255, 255)*intensity; // well duh
        return false;                              // no, we do not discard this pixel
    }
```
-  This routine is called for each pixel inside the triangle we draw; as an input it receives barycentric coordinates for interpolation of varying_ data. Thus, interpolated intensity can be computed as varying_intensity[0]*bar[0]+varying_intensity[1]*bar[1]+varying_intensity[2]*bar[2] or simply as a dot product between two vectors: varying_intensity*bar. In true GLSL, of course, fragment shaders receive ready interpolated values.
-  Notice that the shader returns a bool value. It is easy to understand what it does if we look inside the rasterizer (our_gl.cpp, triangle() function):

```C++
 TGAColor color;
            bool discard = shader.fragment(c, color);
            if (!discard) {
                zbuffer.set(P.x, P.y, TGAColor(P.z));
                image.set(P.x, P.y, color);
            }
```
- Fragment shader can discard drawing of the current pixel, then the rasterizer simply skips it. It is handy if we want to create binary masks or whatever you want (check the lesson 9 for a very cool example of discarding pixels).
- 当然，光栅器不能考虑到各种奇特物体，因此你不能预编译着色器。此处我们使用抽象基类IShader作为两者的中介。我很少使用抽象基类，但是此处不能没有它。指向函数的指针不优雅。

### 着色器的第一次修改
```C++
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;
        if (intensity>.85) intensity = 1;
        else if (intensity>.60) intensity = .80;
        else if (intensity>.45) intensity = .60;
        else if (intensity>.30) intensity = .45;
        else if (intensity>.15) intensity = .30;
        else intensity = 0;
        color = TGAColor(255, 155, 0)*intensity;
        return false;
    }
```
- 基于Gourad着色器的简单修改，intensity允许有六个层次，结果如下：
<img src="E:\GameLearning\tinyRenderer\Notes\CH06\04.png" style="zoom:60%" div align="center"> 

### 纹理
- 我暂时跳过Phone着色器（在第8节会涉及，可以查看LearnOpenGL教程网光照部分）。还记得上次的作业吗？如何插值UV坐标？通过重心坐标。下边有一个2\*3的矩阵，2行是UV,3列是（3个顶点）。
```C++
struct Shader : public IShader {
    Vec3f          varying_intensity; // written by vertex shader, read by fragment shader
    mat<2,3,float> varying_uv;        // same as above

    virtual Vec4f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        return Viewport*Projection*ModelView*gl_Vertex; // transform it to screen coordinates
    }
    
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
        Vec2f uv = varying_uv*bar;                 // interpolate uv for the current pixel
        color = model->diffuse(uv)*intensity;      // well duh
        return false;                              // no, we do not discard this pixel
    }
};
```
-  结果如下：
<img src="E:\GameLearning\tinyRenderer\Notes\CH06\05.png" style="zoom:60%" div align="center"> 

### 法线贴图
- 上边我们通过纹理坐标使用了纹理贴图。那么我们可以在纹理中存什么？事实上，几乎所有。可以是颜色，方向，温度等等，我们看一下下边的纹理
<img src="E:\GameLearning\tinyRenderer\Notes\CH06\06.png" style="zoom:60%" div align="center"> 
- 如果我们把RGB解析为xyz坐标，那么这张图片可以给我们每个像素的法向量，不仅仅是顶点的。
- 顺便，把下边的图和上边的比较下，它同样给出相关信息，差别见补充内容
<img src="E:\GameLearning\tinyRenderer\Notes\CH06\07.png" style="zoom:60%" div align="center">
- 第一张图片给出的法向量是全局坐标，而另一张是切线空间。切线空间z分量是垂直与物体的，x是切线方向，y是它们两个的叉乘。（ In Darboux frame the z-vector is normal to the object, x - principal curvature direction and y - their cross product.）
- 问题1：你能区分哪个是切线空间的，哪个是全局坐标的？
- 问题2：那哪个较好呢？为什么？
```C++
struct Shader : public IShader {
    mat<2,3,float> varying_uv;  // same as above
    mat<4,4,float> uniform_M;   //  Projection*ModelView
    mat<4,4,float> uniform_MIT; // (Projection*ModelView).invert_transpose()

    virtual Vec4f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        return Viewport*Projection*ModelView*gl_Vertex; // transform it to screen coordinates
   }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec2f uv = varying_uv*bar;                 // interpolate uv for the current pixel
        Vec3f n = proj<3>(uniform_MIT*embed<4>(model->normal(uv))).normalize();
        Vec3f l = proj<3>(uniform_M  *embed<4>(light_dir        )).normalize();
        float intensity = std::max(0.f, n*l);
        color = model->diffuse(uv)*intensity;      // well duh
        return false;                              // no, we do not discard this pixel
    }
};
[...]
    Shader shader;
    shader.uniform_M   =  Projection*ModelView;
    shader.uniform_MIT = (Projection*ModelView).invert_transpose();
    for (int i=0; i<model->nfaces(); i++) {
        Vec4f screen_coords[3];
        for (int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }
```
- uniform也是GLSL的预留关键字，允许向着色器中传递常量。Here I pass the matrix Projection\*ModelView and its inverse transpose to transform the normal vectors (refer to the end of the lesson 5). So, computation of the lighting intensity is the same as before with one exception: instead of interpolating normal vectors we retrieve the information from the normal mapping texture (do not forget to transform light vector and normal vectors).
<img src="E:\GameLearning\tinyRenderer\Notes\CH06\08.png" style="zoom:60%" div align="center">

### 镜面贴图
- 好了，我们继续娱乐。计算机图形学就是欺骗的艺术。为了更好的“瞒”过眼睛，我们采用了Phone光照模型。Phong proposed to consider the final lighting as a (weighted) sum of three light intensities: ambient lighting (constant per scene), diffuse lighting (the one we computed up to this moment) and specular lighting.
<img src="E:\GameLearning\tinyRenderer\Notes\CH06\09.png" style="zoom:100%" div align="center"> 
- 漫反射光照，我们是通过法向量和光方向向量的点积。那镜面光呢？
<img src="E:\GameLearning\tinyRenderer\Notes\CH06\10.png" style="zoom:100%" div align="center"> 
- 漫反射我们通过向量**n**和向量**l**计算，而现在我们着重看向量**r**和向量**v**
- 问题3：给出向量**n**和向量**l**，如何找到向**r**？
- 如果**n**和**l**都是单位向量，那么**r**=2**n**<**n**,**l**>-**l**
- 漫反射结果其实就是上边计算的intensity。但是光滑表面的发射要强于其他。Recall that all numbers inferior to 1 will decrease when we apply the power. It means that tenth power of the cosine will give smaller radius of the reflected beam. And hundredth power much smaller beam radius. This power is stored in a special texture (specular mapping texture) that tells for each point if it is glossy or not.
```C++
struct Shader : public IShader {
    mat<2,3,float> varying_uv;  // same as above
    mat<4,4,float> uniform_M;   //  Projection*ModelView
    mat<4,4,float> uniform_MIT; // (Projection*ModelView).invert_transpose()

    virtual Vec4f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        return Viewport*Projection*ModelView*gl_Vertex; // transform it to screen coordinates
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec2f uv = varying_uv*bar;
        Vec3f n = proj<3>(uniform_MIT*embed<4>(model->normal(uv))).normalize();
        Vec3f l = proj<3>(uniform_M  *embed<4>(light_dir        )).normalize();
        Vec3f r = (n*(n*l*2.f) - l).normalize();   // reflected light
        float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
        float diff = std::max(0.f, n*l);
        TGAColor c = model->diffuse(uv);
        color = c;
        for (int i=0; i<3; i++) color[i] = std::min<float>(5 + c[i]*(diff + .6*spec), 255);
        return false;
    }
};
```
- I took 5 for the ambient component, 1 for the diffuse component and .6 for the specular component. What coefficients to choose - is your choice. Different choices give different appearances for the object. Normally it is for the artist to decide.
- Please note that normally the sum of the coefficents must be equal to 1, but you know. I like to create light.
<img src="E:\GameLearning\tinyRenderer\Notes\CH06\11.png" style="zoom:60%" div align="center"> 
<img src="E:\GameLearning\tinyRenderer\Notes\CH06\12.png" style="zoom:60%" div align="center"> 