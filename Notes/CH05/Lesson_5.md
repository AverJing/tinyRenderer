[Lesson 5](https://github.com/ssloy/tinyrenderer/wiki/Lesson-5:-Moving-the-camera)：摄像机

### Last necessary bit of geometry（几何最后的知识点）

- 一旦你掌握今天学习的内容，下一节就可以开始实际渲染。为了激励你，我们先看一下使用Gouraud着色的效果
<img src="E:\GameLearning\tinyRenderer\Notes\CH05\01.png" style="zoom:60%" div align="center">
- 我没有加载纹理。Gouraud着色相当简单，模型每个顶点都有其对应的法向量，可以在"vn x y z"行找到。我们可以计算每个顶点的intensity（和光照），而不是之间每个三角形来计算。之后就像z-buffer和uv坐标那样，通过插值来获取intensity。
- 当然，如果模型没有提供，那么我们也可以重新计算，一个顶点附属的所有平面的法向量的平均值。

### 3D空间的变换（坐标空间）
- 在欧几里得空间中，坐标可以根据一个顶点和基确定。即一个点P(x, y, z)是以(O, i, j, k)为基的。那么向量OP可以写成下边的形式：
<img src="E:\GameLearning\tinyRenderer\Notes\CH05\02.png" style="zoom:100%" div align="center">
- 考虑一下，如果我们换另一个基 到(O', i',j',k')呢？我们如何从原来的坐标转换到现在的坐标？首先我们要意识到从(O, i, j, k)到(O', i',j',k')，存在一个矩阵M，使得：
<img src="E:\GameLearning\tinyRenderer\Notes\CH05\03.png" style="zoom:100%" div align="center">
- 插图如下：
<img src="E:\GameLearning\tinyRenderer\Notes\CH05\04.png" style="zoom:100%" div align="center">
- 重新写向量OP表达式
<img src="E:\GameLearning\tinyRenderer\Notes\CH05\05.png" style="zoom:100%" div align="center">
- 改写为(O, i, j, k)形式，如下
<img src="E:\GameLearning\tinyRenderer\Notes\CH05\06.png" style="zoom:100%" div align="center">
- 当然两者可以相互转换
<img src="E:\GameLearning\tinyRenderer\Notes\CH05\07.png" style="zoom:100%" div align="center">

### 创建LookAt矩阵
- 目前，我们的渲染器只能渲染将摄像机固定在z轴上的画面。如果我们想移动摄像机，我们可以移动画面，让摄像机固定。

- 我们可以这样考虑问题：we want to draw a scene with a camera situated in point e (eye), the camera should be pointed to the point c (center) in such way that a given vector u (up) is to be vertical in the final render. 如下图所示：
  <img src="E:\GameLearning\tinyRenderer\Notes\CH05\08.png" style="zoom:100%" div align="center">

- 这意味着我们想要以 (c, x',y',z')为基，但是我们的模型是以(O, x,y,z)为基的。 没问题，我们只需要进行坐标变换。

  ```c++
  void lookat(Vec3f eye, Vec3f center, Vec3f up) {
      Vec3f z = (eye-center).normalize();
      Vec3f x = cross(up,z).normalize();
      Vec3f y = cross(z,x).normalize();
      Matrix Minv = Matrix::identity();
      Matrix Tr   = Matrix::identity();
      for (int i=0; i<3; i++) {
          Minv[0][i] = x[i];
          Minv[1][i] = y[i];
          Minv[2][i] = z[i];
          Tr[i][3] = -center[i];
      }
      ModelView = Minv*Tr;
  }
  ```
  - 注意上边z，x，y向量的求法（一定要记得单位化）。注意up并不一定和zox面垂直，所以最好在求下y。（这个就是OpenGL中的LookAt矩阵）。

### ViewPOrt（视口变换）
- 如果你学习过前边的教程，你肯定记得下边的代码：
```C++
screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.);
```
- 那它是什么意思？它的意思是，一个二维向量v，它的xy范围属于[-1,1]*[-1,1]，我想把它华仔一个宽width，高height的图片中。Value (v.x+1) is varying between 0 and 2, (v.x+1)/2 between 0 and 1, and (v.x+1)*width/2 sweeps all the image. Thus we effectively mapped the bi-unit square onto the image.
- 把它转为矩阵的形式，如下
```C++
Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = depth/2.f;

    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
    return m;
}
```
- 矩阵如下
  <img src="E:\GameLearning\tinyRenderer\Notes\CH05\09.png" style="zoom:100%" div align="center">
- It means that the bi-unit cube [-1,1]*[-1,1]*[-1,1] is mapped onto the screen cube [x,x+w]*[y,y+h]*[0,d]. Right, cube, and not a rectangle, this is because of the depth computations with the z-buffer. Here d is the resolution of the z-buffer. I like to have it equal to 255 because of simplicity of dumping black-and-white images of the z-buffer for debugging.

In the OpenGL terminology this matrix is called viewport matrix.

### 一系列的坐标变换
- 总结：模型是创造在局部坐标下的。我们需要将其在世界坐标下展示。从局部坐标到世界坐标的转换是Model矩阵。之后我们想在摄像机空间展示，变换矩阵是View。之后我们用Projection矩阵进行透视变换，此时坐标系叫做剪裁坐标。最终我们就渲染画面，我们要将剪裁坐标转变为屏幕坐标，称为视口变换（ViewPort）。
- 如果我们再从obj文件读取信息，那么将对顶点施加一系列变换
```C++
Viewport * Projection * View * Model * v.

Vec3f v = model->vert(face[j]);
screen_coords[j] =  Vec3f(ViewPort*Projection*ModelView*Matrix(v));
```

### 对法向量的变换
- 如果一个模型提供了法向量，并且该模型经过仿射变换，那么变换后的法向量等于变换矩阵的逆的转置（或者转置的逆）和原始法向量相乘。
- 推导的过程并不复杂。3D图形学与数学书中有，P49
