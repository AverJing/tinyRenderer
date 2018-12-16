[Lesson 7](https://github.com/ssloy/tinyrenderer/wiki/Lesson-7:-Shadow-mapping)：阴影贴图

### 目标

- 简短的CG课程快要结束了，但我希望这只是个开始，我只是坑的领路人，后边还有无数的深坑等着大家跳（自娱自乐）。今天的目标是实现阴影贴图。注意，我们此处讨论的是hard shadows，而不是soft shadow。
<img src="01.png" style="zoom:60%" div align="center">

### 问题描述
- 我们使用局部渲染可以正确渲染出凸多边形。局部意味着我们使用光照向量和法向量来计算。可是，这对于凹多边形不起作用。下边这张图就是我们从前边课程渲染的结果。
<img src="02.png" style="zoom:60%" div align="center">
- 为什么在模型右侧肩膀处有光照？为什么我们看不到阴影？
- 想法很简单：我们分两步走：
	- 1. 我们先把摄像机放在光源位置渲染该图像（想得到z-buffer），这一步可以决定那部分有阴影。
	- 2. 根据上边得到的z-buffer，正常位置渲染图像

```C++
struct DepthShader : public IShader {
    mat<3,3,float> varying_tri;

    DepthShader() : varying_tri() {}

    virtual Vec4f vertex(int iface, int nthvert) {
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        gl_Vertex = Viewport*Projection*ModelView*gl_Vertex;          // transform it to screen coordinates
        varying_tri.set_col(nthvert, proj<3>(gl_Vertex/gl_Vertex[3]));
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec3f p = varying_tri*bar;
        color = TGAColor(255, 255, 255)*(p.z/depth);
        return false;
    }
};
```
- 着色器将z-buffer结果拷贝到framebuffer。下边是我如何从主函数使用它：
```C++
     { // rendering the shadow buffer
        TGAImage depth(width, height, TGAImage::RGB);
        lookat(light_dir, center, up);
        viewport(width/8, height/8, width*3/4, height*3/4);
        projection(0);

        DepthShader depthshader;
        Vec4f screen_coords[3];
        for (int i=0; i<model->nfaces(); i++) {
            for (int j=0; j<3; j++) {
                screen_coords[j] = depthshader.vertex(i, j);
            }
            triangle(screen_coords, depthshader, depth, shadowbuffer);
        }
        depth.flip_vertically(); // to place the origin in the bottom left corner of the image
        depth.write_tga_file("depth.tga");
    }

    Matrix M = Viewport*Projection*ModelView;
```
- 我将摄像机放在光源方向(lookat(light_dir, center, up);)，然后执行渲染。注意我保存z-buffer，有shadowbuffer指向它。还要注意，我保存了从object-screen的变换矩阵M。depth结果如下图：
<img src="03.png" style="zoom:60%" div align="center">
- 很自然，第二步由另外一个着色器实现。
```C++
struct Shader : public IShader {
    mat<4,4,float> uniform_M;   //  Projection*ModelView
    mat<4,4,float> uniform_MIT; // (Projection*ModelView).invert_transpose()
    mat<4,4,float> uniform_Mshadow; // transform framebuffer screen coordinates to shadowbuffer screen coordinates
    mat<2,3,float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
    mat<3,3,float> varying_tri; // triangle coordinates before Viewport transform, written by VS, read by FS

    Shader(Matrix M, Matrix MIT, Matrix MS) : uniform_M(M), uniform_MIT(MIT), uniform_Mshadow(MS), varying_uv(), varying_tri() {}

    virtual Vec4f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        Vec4f gl_Vertex = Viewport*Projection*ModelView*embed<4>(model->vert(iface, nthvert));
        varying_tri.set_col(nthvert, proj<3>(gl_Vertex/gl_Vertex[3]));
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec4f sb_p = uniform_Mshadow*embed<4>(varying_tri*bar); // corresponding point in the shadow buffer
        sb_p = sb_p/sb_p[3];
        int idx = int(sb_p[0]) + int(sb_p[1])*width; // index in the shadowbuffer array
        float shadow = .3+.7*(shadowbuffer[idx]<sb_p[2]); 
        Vec2f uv = varying_uv*bar;                 // interpolate uv for the current pixel
        Vec3f n = proj<3>(uniform_MIT*embed<4>(model->normal(uv))).normalize(); // normal
        Vec3f l = proj<3>(uniform_M  *embed<4>(light_dir        )).normalize(); // light vector
        Vec3f r = (n*(n*l*2.f) - l).normalize();   // reflected light
        float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
        float diff = std::max(0.f, n*l);
        TGAColor c = model->diffuse(uv);
        for (int i=0; i<3; i++) color[i] = std::min<float>(20 + c[i]*shadow*(1.2*diff + .6*spec), 255);
        return false;
    }
};
```
- 这只是上一节课某个shader的拷贝，除了一点：我声明了一个名为uniform_Mshadow的矩阵，它的作用是将当前屏幕坐标转换到shadowbuffer中的屏幕坐标。在后边我会解释如何计算的，我们先看下如何使用它：
```C++
     Vec4f sb_p = uniform_Mshadow*embed<4>(varying_tri*bar); // corresponding point in the shadow buffer
        sb_p = sb_p/sb_p[3];
        int idx = int(sb_p[0]) + int(sb_p[1])*width; // index in the shadowbuffer array
        float shadow = .3+.7*(shadowbuffer[idx]<sb_p[2]);
```
- varying_tri\*bar给我们提供当前画的像素点的屏幕坐标。我们扩充一维（回忆下齐次坐标），之后使用“魔性变换”（上边提到了）。现在我们知道sb_p的坐标是在shadowbuffer空间下。接下来，我们要决定当前像素是否在阴影当中（根据当前z分量和shadowbuffer中存的值比较）。
- 看一下我是如何调用这个shader的：
```C++
    Matrix M = Viewport*Projection*ModelView;

    { // rendering the frame buffer
        TGAImage frame(width, height, TGAImage::RGB);
        lookat(eye, center, up);
        viewport(width/8, height/8, width*3/4, height*3/4);
        projection(-1.f/(eye-center).norm());

        Shader shader(ModelView, (Projection*ModelView).invert_transpose(), M*(Viewport*Projection*ModelView).invert());
        Vec4f screen_coords[3];
        for (int i=0; i<model->nfaces(); i++) {
            for (int j=0; j<3; j++) {
                screen_coords[j] = shader.vertex(i, j);
            }
            triangle(screen_coords, shader, frame, zbuffer);
        }
        frame.flip_vertically(); // to place the origin in the bottom left corner of the image
        frame.write_tga_file("framebuffer.tga");
    }
```
- Recall that the matrix M is the transformation matrix from the object space to the shadow buffer screen space. We return the camera back to its normal position, recompute the viewport matrix, the projection matrix and call the second shader.

- We know that Viewport\*Projection\*ModelView transforms the object's coordinates into the (framebuffer) screen space. We need to know how to transform the framebuffer screen into the shadow screen. It is really simple: (Viewport\*Projection\*ModelView).invert() allows to convert framebuffer coordinates into object coordinates and then M\*(Viewport\*Projection\*ModelView).invert() gives the transformation between the framebuffer and the shadow buffer.
- 结果如下：
<img src="04.png" style="zoom:60%" div align="center">
- Notice the ugly shadow rendering? This artifact is known as the z-fighting. Resolution of our buffers is insufficient to obtain precise results. How to solve the problem? I like brute force solutions:
```C++
float shadow = .3+.7*(shadowbuffer[idx]<sb_p[2]+43.34); // magic coeff to avoid z-fighting
```
- I simply move a bit one z-buffer with respect to another, it is sufficient to remove the artifact. Yes, it creates other problems (can you tell which ones?), but those are generally less visible. The final render is visible in the teaser image.

## 作业

### 阴影贴图
- 试着给下边的图，添加阴影效果
<img src="05.png" style="zoom:60%" div align="center">

### 眼睛发光
- 如何达到这个效果？