[Lesson 6bis](https://github.com/ssloy/tinyrenderer/wiki/Lesson-6bis:-tangent-space-normal-mapping)：切线空间下的法线贴图

### 回顾

- 今天的主题是法线贴图。法线贴图和Phone着色的区别？关键是法线信息的密度。对于Phone着色，我们使用的是一个三角形对应的一个法线向量（有3个顶点可生成），然而法线贴图可以提供更加详细的信息，可以改善渲染细节。
- 上一节我们使用了法线贴图，只不过是全局坐标系统下的。今天我们要讨论的是切线空间的发现贴图。
- 看下边的纹理图，左边是全局的，右边的是切线空间的。
<img src="E:\GameLearning\tinyRenderer\Notes\CH06bis\01.jpg" style="zoom:60%" div align="center">
- 为了使用右边的纹理，我们需要计算每个像素在切线空间下的坐标。以此为基的z向量会和我们的表面垂直，另外两个向量给出了当前顶点的切平面。之后我们从纹理中读入一个法向量，将它的坐标从Darboux转换到全局坐标，结束工作。Usually normal maps provide small perturbations of normal vectors, thus textures are in dominant blue color.
- 为什么要引入切线空间？而不直接使用全局坐标？想想一下我们让模型动起来。例如，我让那个黑色的小伙子张一下嘴巴。很明显，法线向量改变了。
<img src="E:\GameLearning\tinyRenderer\Notes\CH06bis\02.jpg" style="zoom:60%" div align="center">
- 左边的图使用的仍是全局坐标的法线贴图。近距离观察下模型的下嘴唇，发现下嘴唇应该了“亮些”。对比右边用切线空间实现的效果。
- 因此，如果我们想让模型动起来，那么我们就要给出全局空间下正确的法线贴图，这意味着我们需要给出每一帧的法线贴图。而切线空间的法线会自适应（根据模型在计算）。
- 看下边的例子：
<img src="E:\GameLearning\tinyRenderer\Notes\CH06bis\03.jpg" style="zoom:60%" div align="center">
- These are textures for the Diablo model. Notice that only one hand is drawn in the texture, and only one side of the tail. The artist used the same texture for both arms and for both sides of the tail. It means that in the global coordinate system I can provide normal vectors either for the left side of the tail, either for the right one, but not for both! Same goes for the arms. And I need different information for the left and the right sides, for example, inspect left and right cheekbones in the left image, naturally normal vectors are pointing in the opposite directions!

- Let us finish with the motivation section and go straight ahead to the computations.

### 起点，Phone着色
- 看一下Phoneshader
```C++
struct Shader : public IShader {
    mat<2,3,float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
    mat<3,3,float> varying_nrm; // normal per vertex to be interpolated by FS

    virtual Vec4f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        varying_nrm.set_col(nthvert, proj<3>((Projection*ModelView).invert_transpose()*embed<4>(model->normal(iface, nthvert), 0.f)));
        Vec4f gl_Vertex = Projection*ModelView*embed<4>(model->vert(iface, nthvert));
        varying_tri.set_col(nthvert, gl_Vertex);
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec3f bn = (varying_nrm*bar).normalize();
        Vec2f uv = varying_uv*bar;

        float diff = std::max(0.f, bn*light_dir);
        color = model->diffuse(uv)*diff;
        return false;
    }
};
```
- 结果如下：
<img src="E:\GameLearning\tinyRenderer\Notes\CH06bis\04.jpg" style="zoom:60%" div align="center">
- 为了方便debug，将纹理变为规律的红蓝格子（红和蓝垂直）
<img src="E:\GameLearning\tinyRenderer\Notes\CH06bis\05.jpg" style="zoom:60%" div align="center">
- 我们来看一下Phone着色如何工作的：（此部分暂不译，[可以看这](https://learnopengl-cn.github.io/05%20Advanced%20Lighting/04%20Normal%20Mapping/)）
<img src="E:\GameLearning\tinyRenderer\Notes\CH06bis\06.png" style="zoom:60%" div align="center">
- For each vertex of a triangle we have its coordinates p, texture coordinates uv and normal vectors. For shading current fragment our software rasterizer gives us barycentric coordinates of the fragment (alpha, beta, gamma). It means that the coordinates of the fragment can be obtained as p = alpha p0 + beta p1 + gamma p2. Then in the same way we interpolate texture coordinates and the normal vector:
<img src="E:\GameLearning\tinyRenderer\Notes\CH06bis\07.png" style="zoom:100%" div align="center">
- **Notice that blue and red lines are isolines of u and v, correspondingly. So, for each point of our surface we define a so-called Darboux frame, with x and y axes parallel to blue and red lines, and z axis orthogonal to the surface. This is the frame where tangent space normal map lives.**

### How to reconstruct a (3D) linear function from three samples
- Okay, so our goal is to compute three vectors (tangent basis) for each pixel we draw. Let us put that aside for a while and imagine a linear function f that for each point (x,y,z) gives a real number f(x,y,z) = Ax + By + Cz + D. The only problem that we do not know A, B, C and D, however we do know three values of the function at three different points of the space (p0, p1, p2):
<img src="E:\GameLearning\tinyRenderer\Notes\CH06bis\08.png" style="zoom:100%" div align="center">
<img src="E:\GameLearning\tinyRenderer\Notes\CH06bis\09.png" style="zoom:100%" div align="center">
- It is convenient to imagine f as a height map of an inclined plane. We fix three different (non collinear) points on the plane and we know values of f in those points. Red lines inside the triangle show the iso-heights f0, f0 + 1 meter, f0 + 2 meters and so on. For a linear function we have the isolines are parallel (straight) lines.
- In fact, I am more interested in the direction, orthogonal to the isolines. If we move along an iso, the height does not change (well duh, it is an iso!). If we deviate a little bit from an iso, the height starts to change a little bit. The steepest ascent we obtain when we move orthogonally to the isolines.
- Let us recall that the steepest ascent direction for a function is nothing else than its gradient. For a linear function f(x,y,z) = Ax + By + Cz + D its gradient is a constant vector (A, B, C). （关于f函数的理解？千万不要把它当作平面，(A,B,C)是其法向量，并不是而是梯度）Recall that we do not know the values of (A,B,C). We know only three samples of the function. Can we reconstruct A,B and C? Sure thing.
- So, we have three points p0, p1, p2 and thre values f0, f1, f2. We need to find the vector of the steepest ascent (A,B,C). Let us consider another function defined as g(p) = f(p) - f(p0):
<img src="E:\GameLearning\tinyRenderer\Notes\CH06bis\10.png" style="zoom:100%" div align="center">
- Obviously that we simply translated our inclined plane, without changing its inclination, therefore the steepest ascent direction for f and g is the same.
- Let us rewrite the definition of g:
<img src="E:\GameLearning\tinyRenderer\Notes\CH06bis\11.png" style="zoom:100%" div align="center">
- Please note that superscript x in p^x means x coordinate of the point p and not a power. So, the function g is simply a dot product between vector (p-p0) and (ABC). And we still do not know (A,B,C)!
- Okay, let us recall what we know. We know that if we go from point p0 to point p2, then the function g will go from zero to f2-f0. In other words, the dot product between vectors (p2-p0) and (ABC) is equal to f2-f0. Same thing for (p1-p0). Therefore, we are looking for the vector ABC, orthogonal to the normal n and respecting two constraints on dot products:
<img src="E:\GameLearning\tinyRenderer\Notes\CH06bis\12.png" style="zoom:100%" div align="center">
- Let us rewrite this in a matrix form:
<img src="E:\GameLearning\tinyRenderer\Notes\CH06bis\13.png" style="zoom:100%" div align="center">
- So, we got an easy to solve linear matrix equation Ax = b:
<img src="E:\GameLearning\tinyRenderer\Notes\CH06bis\14.png" style="zoom:100%" div align="center">
- Please note that I used the letter A for two different things, the meaning should be clear from the context. So, our 3x3 matrix A, multiplied with the unknown vector x=(A,B,C), gives the vector b = (f1-f0, f2-f0, 0). Unknown vector x becomes known when we multiply inverse to A by b.
- Also note that in the matrix A we have nothing related to the function f. It contains only some information about our triangle.

### Let us compute Darboux basis and apply the perturbation of normals
- So, Darboux basis is a triplet of vectors (i,j,n), where n - is the original normal vector, and i, j can be computed as follows:
<img src="E:\GameLearning\tinyRenderer\Notes\CH06bis\15.png" style="zoom:100%" div align="center">
- Here is the commit, using the normal maps in the tangent space, and here you can check the differencies with respect to the starting point (Phong shading).
- All is quite straightforward, I compute the matrix A:
```C++
mat<3,3,float> A;
        A[0] = ndc_tri.col(1) - ndc_tri.col(0);
        A[1] = ndc_tri.col(2) - ndc_tri.col(0);
        A[2] = bn;
```
- Then compute two unknown vectors (i,j) of Darboux basis:
```C++
mat<3,3,float> AI = A.invert();
        Vec3f i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
        Vec3f j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);
```
- Once we have all the tangent basis, I read the perturbed normal from the texture and apply the basis change from the tangent basis to the global coordinates. Recall that I have already described how to perform change of basis.
- Here is the final rendered image, compare the details with Phong shading:
<img src="E:\GameLearning\tinyRenderer\Notes\CH06bis\16.jpg" style="zoom:60%" div align="center">

### Debugging advice
- Now it is the perfect time to recall how to draw straight line segments. Apply the red-blue grid as the texture and draw the vectors (i,j) for each vertex of the mesh. Normally they must coincide with the texture lines.

### Were you paying attention?
- Have you noticed that generally a (flat) triangle has a constant normal vector, whereas I used the interpolated normal in the last row of the matrix A? Why did I do it?
- 一个顶点不仅仅作为一个三角形的顶点。