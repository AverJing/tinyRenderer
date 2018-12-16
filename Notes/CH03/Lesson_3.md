[Lesson 3](https://github.com/ssloy/tinyrenderer/wiki/Lesson-3:-Hidden-faces-removal-(z-buffer))：裁剪（z-buffer）

#### 前言

- 大家好，我来给大家介绍我们的朋友z-buffer（一个黑色的家伙）。他会帮助我们消除模型的背面。<img src="E:\GameLearning\tinyRenderer\Notes\CH03\01.png" style="zoom:60%" div align="center">
- 感谢Vidar Rapp提供的黑人模型。上一节我们把所有的三角形都画出来了，没有进行丢弃操作。如果我们恰当的从后到前渲染，那前边的面就会覆盖后边的。这种方法称为**画家算法**。不幸的是，它常常会带来较高的计算消耗，每一次相机移动，我们需要重新排序整个画面，同时还存在动态画面。这还不是主要问题，最要命的是它并不总是能够正确的决定顺序。

### 尝试渲染一个简单的画面
- 想象一个简单的场景：由三个不同颜色的三角形拼成的，摄像机从上到下，我们将颜色投影在一个白色的平面上。
  <img src="E:\GameLearning\tinyRenderer\Notes\CH03\02.png" style="zoom:60%" div align="center">
- 看起来这样 
  <img src="E:\GameLearning\tinyRenderer\Notes\CH03\03.png" style="zoom:60%" div align="center">
- Blue facet - is it behind or in front of the red one? The painter's algorithm does not work here. It is possible to split blue facet in two (one in front of the red facet and one behind). And then the one in front of the red one is to be split in two - one in front of the green triangle and one behind... I think you get the problem: in scenes with millions of triangles it is really expensive to compute. It is possible to use [BSP trees](https://en.wikipedia.org/wiki/Binary_space_partitioning) to get it done. By the way, this data structure is constant for moving camera, but it is really messy. And the life is too short to get it messy.

#### 简单的尝试，从y-buffer开始

- 先从二维开始，用黄色平面把图像切割，如下图
  <img src="E:\GameLearning\tinyRenderer\Notes\CH03\04.png" style="zoom:60%" div align="center">

- 如上图，我们的画面现在又三条线段组成（三个三角形和黄色平面相交），最后的渲染结果限制在一个像素高度。
  <img src="E:\GameLearning\tinyRenderer\Notes\CH03\05.png" style="zoom:60%" div align="center">

- 我们的画面现在是二维的，使用line函数可以很简单的实现。

  ```c++
      { // just dumping the 2d scene (yay we have enough dimensions!)
          TGAImage scene(width, height, TGAImage::RGB);
  
          // scene "2d mesh"
          line(Vec2i(20, 34),   Vec2i(744, 400), scene, red);
          line(Vec2i(120, 434), Vec2i(444, 400), scene, green);
          line(Vec2i(330, 463), Vec2i(594, 200), scene, blue);
  
          // screen line
          line(Vec2i(10, 10), Vec2i(790, 10), scene, white);
  
          scene.flip_vertically(); // i want to have the origin at the left bottom corner of the image
          scene.write_tga_file("scene.tga");
      }
  ```

- 结果如下图所示
  <img src="E:\GameLearning\tinyRenderer\Notes\CH03\06.png" style="zoom:60%" div align="center">

- 那我们开始渲染把。回忆上边说的一个像素高度，但是下边的实现是16个像素高度。

  ```c++
  TGAImage render(width, 16, TGAImage::RGB);
          int ybuffer[width];
          for (int i=0; i<width; i++) {
              ybuffer[i] = std::numeric_limits<int>::min();
          }
          rasterize(Vec2i(20, 34),   Vec2i(744, 400), render, red,   ybuffer);
          rasterize(Vec2i(120, 434), Vec2i(444, 400), render, green, ybuffer);
          rasterize(Vec2i(330, 463), Vec2i(594, 200), render, blue,  ybuffer);
  ```

- 注意上边那个神奇数组ybuffer。数组中的默认值由负无穷初始化。那rasterize()是如何工作的？

  ```c++
  void rasterize(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int ybuffer[]) {
      if (p0.x>p1.x) {
          std::swap(p0, p1);
      }
      for (int x=p0.x; x<=p1.x; x++) {
          float t = (x-p0.x)/(float)(p1.x-p0.x);
          int y = p0.y*(1.-t) + p1.y*t;
          if (ybuffer[x]<y) {
              ybuffer[x] = y;
              image.set(x, 0, color);
          }
      }
  }
  ```
- 相当简单，我先从p0.x到p1.x迭代x，并计算相应的y坐标轴。之后我会检查当前x下标下的ybuffer。如果当前y值是更接近摄像机（比ybuffer中的值小），那么就将它绘制到屏幕上并更新ybuffer。
- 让我们一步一步看。rasterize红色的线时，
screen
<img src="E:\GameLearning\tinyRenderer\Notes\CH03\07.png" style="zoom:60%" div align="center">
ybuffer
<img src="E:\GameLearning\tinyRenderer\Notes\CH03\08.png" style="zoom:60%" div align="center">
- 两头的品红代表负无穷，这意味着这部分的平面坐标我们没有设置。颜色越暗离摄像机越远。
- 接下来我们画绿色的
screen
<img src="E:\GameLearning\tinyRenderer\Notes\CH03\09.png" style="zoom:60%" div align="center">
ybuffer
<img src="E:\GameLearning\tinyRenderer\Notes\CH03\10.png" style="zoom:60%" div align="center">
- 最终结果
screen
<img src="E:\GameLearning\tinyRenderer\Notes\CH03\1.png" style="zoom:60%" div align="center">
ybuffer
<img src="E:\GameLearning\tinyRenderer\Notes\CH03\12.png" style="zoom:60%" div align="center">
- 恭喜，我们如愿完成了2D和1D画面。我们再看一下最终的结果（注意按照作者的意思图片y轴自图片上而下增加的）
<img src="E:\GameLearning\tinyRenderer\Notes\CH03\12.png" style="zoom:60%" div align="center">

#### 回到3D
- 为了画2D图形，zbuffer必须是二维的。```int *zbuffer = new int[width*height];```

- 个人而言，我将二维数组将为一维数组，相互访问也不困难```int idx = x + y*width;``` 

  ```C++
  int x = idx % width;
  int y = idx / width;
  ```
- 困难在于如何计算z-buffer。我们先回忆我们如何计算y-buffer的。
```C++
   int y = p0.y*(1.-t) + p1.y*t;
```
- **t变量的本质是什么？** **It turns out that (1-t, t) are barycentric coordinates of the point (x,y) with respect to the segment p0, p1: (x,y) = p0*(1-t) + p1*t.** 所以思路就是我们采用重心坐标的三角形光栅化。对于我们要画的每一个像素，我们用该三角形的三个顶点的z坐标和该像素点的重心坐标相乘。

  ```C++
  triangle(screen_coords, float *zbuffer, image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
  
  [...]
  
  void triangle(Vec3f *pts, float *zbuffer, TGAImage &image, TGAColor color) {
      Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
      Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
      Vec2f clamp(image.get_width()-1, image.get_height()-1);
      for (int i=0; i<3; i++) {
          for (int j=0; j<2; j++) {
              bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]));
              bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
          }
      }
      Vec3f P;
      for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
          for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
              Vec3f bc_screen  = barycentric(pts[0], pts[1], pts[2], P);
              if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
              P.z = 0;
              for (int i=0; i<3; i++) P.z += pts[i][2]*bc_screen[i];
              if (zbuffer[int(P.x+P.y*width)]<P.z) {
                  zbuffer[int(P.x+P.y*width)] = P.z;
                  image.set(P.x, P.y, color);
              }
          }
      }
  }
  ```
- 简单修改上一节丢弃隐藏部分的代码，可以得到下边的效果。
<img src="E:\GameLearning\tinyRenderer\Notes\CH03\14.png" style="zoom:60%" div align="center">

#### 我们刚才通过重心坐标计算了z-buffer，还能干什么？
- 纹理其实也可以采取同样的操作。
- In the .obj file we have lines starting with "vt u v", they give an array of texture coordinates. The number in the middle (between the slashes) in the facet lines "f x/x/x x/x/x x/x/x" are the texture coordinates of this vertex of this triangle. Interpolate it inside the triangle, multiply by the width-height of the texture image and you will get the color to put in your render.