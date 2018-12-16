### [Lesson 2](https://github.com/ssloy/tinyrenderer/wiki/Lesson-2:-Triangle-rasterization-and-back-face-culling)：三角形光栅化和背面剔除



#### 填充三角形

- <img src="E:\GameLearning\tinyRenderer\Notes\CH02\01.png" style="zoom:60%" div align="center">
- 作者自画像
- 上一节我们渲染了一个3D物体的框架。这一次我们将填充多边形而不仅仅三角形。事实上，三角形可以组成复杂的多边形。（这些模型其实都是由三角形拼成的）
- 温馨提醒，这一系列文章是用来锻炼你自己的编程能力的。作者提供的代码只是参考。

#### 传统方法：线性扫描

- 任务是画一个二维三角形。可不是边框的，而是用线完整填满整个三角形。有趣的挑战，实现并不困难。有些同学不仔细分析，写出下边代码

```c++
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) { 
    line(t0, t1, image, color); 
    line(t1, t2, image, color); 
    line(t2, t0, image, color); 
}

// ...

Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)}; 
Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)}; 
Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)}; 
triangle(t0[0], t0[1], t0[2], image, red); 
triangle(t1[0], t1[1], t1[2], image, white); 
triangle(t2[0], t2[1], t2[2], image, green);
```
- <img src="E:\GameLearning\tinyRenderer\Notes\CH02\02.png" style="zoom:100%" div align="center">

- 上边代码仅仅画出了三角形的轮廓（contour），但是我们应该如何画一个完整三角形？一个可行的方法必须包含下边的特点：
  - 它应该足够简单，快速
  - 它应该是对称的，结果不应该依赖传递顶点的顺序
  - 如果两个三角形有相同的顶点，那么它们应该是重合的
  - 我们还可以增加更多的要求，但让我们先解决这个问题。通常线性扫描的过程如下：
    - 将三角形的顶点按照y轴排序
    - 同时光栅化三角形的左边和右边（将三角形分为两个底边和x轴平行）
    - 从左边界向右边界画水平直线

- 哪边是左，哪边是右？切记读作者的代码，不如将自己的代码和他的比较。（思考，，，，）

- 我如何画三角形？我们假定三角形的三个顶点为：t0，t1，t2，它们已经按照y轴排好序。t0和t2是边界A，t0和t1，t2和t1是边界B，如下图<img src="E:\GameLearning\tinyRenderer\Notes\CH02\03.png" style="zoom:100%" div align="center">

- 红色代表边界A，绿色代表边界B。不幸的是，边界B由两部分组成，我们从水平方向将三角形分割成两个部分。

  ```c++
  void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) { 
      // sort the vertices, t0, t1, t2 lower−to−upper (bubblesort yay!) 
      if (t0.y>t1.y) std::swap(t0, t1); 
      if (t0.y>t2.y) std::swap(t0, t2); 
      if (t1.y>t2.y) std::swap(t1, t2); 
      int total_height = t2.y-t0.y; 
      for (int y=t0.y; y<=t1.y; y++) { 
          int segment_height = t1.y-t0.y+1; 
          float alpha = (float)(y-t0.y)/total_height; 
          float beta  = (float)(y-t0.y)/segment_height; // be careful with divisions by zero 
          Vec2i A = t0 + (t2-t0)*alpha; 
          Vec2i B = t0 + (t1-t0)*beta; 
          image.set(A.x, y, red); 
          image.set(B.x, y, green); 
      } 
  }
  ```
  <img src="E:\GameLearning\tinyRenderer\Notes\CH02\04.png" style="zoom:100%" div align="center"/>

  - 注意某些段不是连续的。还记得上一节的xy交换？我们尝试填充水平直线。

  - ```c++
    void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) { 
        // sort the vertices, t0, t1, t2 lower−to−upper (bubblesort yay!) 
        if (t0.y>t1.y) std::swap(t0, t1); 
        if (t0.y>t2.y) std::swap(t0, t2); 
        if (t1.y>t2.y) std::swap(t1, t2); 
        int total_height = t2.y-t0.y; 
        for (int y=t0.y; y<=t1.y; y++) { 
            int segment_height = t1.y-t0.y+1; 
            float alpha = (float)(y-t0.y)/total_height; 
            float beta  = (float)(y-t0.y)/segment_height; // be careful with divisions by zero 
            Vec2i A = t0 + (t2-t0)*alpha; 
            Vec2i B = t0 + (t1-t0)*beta; 
            if (A.x>B.x) std::swap(A, B); 
            for (int j=A.x; j<=B.x; j++) { 
                image.set(j, y, color); // attention, due to int casts t0.y+i != A.y 
            } 
        } 
        for (int y=t1.y; y<=t2.y; y++) { 
            int segment_height =  t2.y-t1.y+1; 
            float alpha = (float)(y-t0.y)/total_height; 
            float beta  = (float)(y-t1.y)/segment_height; // be careful with divisions by zero 
            Vec2i A = t0 + (t2-t0)*alpha; 
            Vec2i B = t1 + (t2-t1)*beta; 
            if (A.x>B.x) std::swap(A, B); 
            for (int j=A.x; j<=B.x; j++) { 
                image.set(j, y, color); // attention, due to int casts t0.y+i != A.y 
            } 
        } 
    }
    ```
    <img src="E:\GameLearning\tinyRenderer\Notes\CH02\05.png" style="zoom:100%" div align="center"/>

    - 上边可以得到结果，但是作者将两部分合并。虽然这样做会缺少可读性

    - ```C++
      void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) { 
          if (t0.y==t1.y && t0.y==t2.y) return; // I dont care about degenerate triangles 
          // sort the vertices, t0, t1, t2 lower−to−upper (bubblesort yay!) 
          if (t0.y>t1.y) std::swap(t0, t1); 
          if (t0.y>t2.y) std::swap(t0, t2); 
          if (t1.y>t2.y) std::swap(t1, t2); 
          int total_height = t2.y-t0.y; 
          for (int i=0; i<total_height; i++) { 
              bool second_half = i>t1.y-t0.y || t1.y==t0.y; 
              int segment_height = second_half ? t2.y-t1.y : t1.y-t0.y; 
              float alpha = (float)i/total_height; 
              float beta  = (float)(i-(second_half ? t1.y-t0.y : 0))/segment_height; // be careful: with above conditions no division by zero here 
              Vec2i A =               t0 + (t2-t0)*alpha; 
              Vec2i B = second_half ? t1 + (t2-t1)*beta : t0 + (t1-t0)*beta; 
              if (A.x>B.x) std::swap(A, B); 
              for (int j=A.x; j<=B.x; j++) { 
                  image.set(j, t0.y+i, color); // attention, due to int casts t0.y+i != A.y 
              } 
          } 
      }
      ```
<img src="E:\GameLearning\tinyRenderer\Notes\CH02\06.jpg" style="zoom:60%" div align="center"/>
- 上边的代码可以借助这张图来理解，根据三角形相似求解。

 #### 下边的方法才是重中之重

- 上边的代码不复杂，但是比较混乱。结合我给出的图来理解。上边的方法是旧时的单一CPU渲染。我们先欣赏一段伪码

  ```c++
  triangle(vec2 points[3]) { 
      vec2 bbox[2] = find_bounding_box(points); 
      for (each pixel in the bounding box) { 
          if (inside(points, pixel)) { 
              put_pixel(pixel); 
          } 
      } 
  }
  //现代三角形画法，支持并行计算。
  ```

- 伪码极具表现力，简单明了。接下来我们看下细节。难点在如何检查一个点在三角形内部？

- *Off Topic: if I have to implement some code to check whether a point belongs to a polygon, and this program will run on a plane, I will never get on this plane. Turns out, it is a surprisingly difficult task to solve this problem reliably. But here we just painting pixels. I am okay with that.*

- 新手看见伪码会有很大的热情，经验丰富的程序员会偷笑，这写的是什么？图形学编程的专家会耸耸肩膀，说到，这就是它如何工作的。（大道至简，从简单到复杂再到简单，这是蜕变。）

- 好了，我们先从重心坐标开始（有一维的，二维的，三维的）。给一个2D三角形ABC以及一个点P，它们都处于笛卡尔坐标。我们的目标是找到P关于三角形ABC的重心坐标。这意味着我们需要三个成员(1-u-v, u, v)，等式如下：
  <img src="E:\GameLearning\tinyRenderer\Notes\CH02\07.png" style="zoom:100%" div align="center"/>

- 把(1-u-v, u, v)当作三角形顶点A，B和C的权重。改写为向量的形式，如下：
  <img src="E:\GameLearning\tinyRenderer\Notes\CH02\08.png" style="zoom:100%" div align="center"/>
  <img src="E:\GameLearning\tinyRenderer\Notes\CH02\09.png" style="zoom:100%" div align="center"/>

- 我们把x和y分开，写成两个各自的等式，得到：
  <img src="E:\GameLearning\tinyRenderer\Notes\CH02\10.png" style="zoom:100%" div align="center"/>

- 改写为矩阵的形式：
  <img src="E:\GameLearning\tinyRenderer\Notes\CH02\11.png" style="zoom:100%" div align="center"/>

- 这意味着我们在寻找一个向量(u, v, 1)分别和$$(AB_x, AC_x, PA_x)$$和$$(AB_y, AC_y, PA_y)$$。（这不就是叉乘吗？）测试一下你，如何通过两个点找到过这两点的直线的表达式？

- 好了，我们来编写新的渲染方案： 

  - 给定一个三角形，用一个矩形刚刚包围这个三角形。
  - 对于这个边界框，我们计算在其中的每个像素的重心坐标。
  - 如果(1-u-v, u, v)至少有一个是负的，那么这个像素就不在三角形内。那我们直接来看代码把

  ```c++
  #include <vector> 
  #include <iostream> 
  #include "geometry.h"
  #include "tgaimage.h" 
   
  const int width  = 200; 
  const int height = 200; 
   
  Vec3f barycentric(Vec2i *pts, Vec2i P) { 
      Vec3f u = cross(Vec3f(pts[2][0]-pts[0][0], pts[1][0]-pts[0][0], pts[0][0]-P[0]), Vec3f(pts[2][1]-pts[0][1], pts[1][1]-pts[0][1], pts[0][1]-P[1]));
      /* `pts` and `P` has integer value as coordinates
         so `abs(u[2])` < 1 means `u[2]` is 0, that means
         triangle is degenerate, in this case return something with negative coordinates */
      if (std::abs(u[2])<1) return Vec3f(-1,1,1);
      return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z); 
  } 
   
  void triangle(Vec2i *pts, TGAImage &image, TGAColor color) { 
      Vec2i bboxmin(image.get_width()-1,  image.get_height()-1); 
      Vec2i bboxmax(0, 0); 
      Vec2i clamp(image.get_width()-1, image.get_height()-1); 
      for (int i=0; i<3; i++) { 
          for (int j=0; j<2; j++) { 
              bboxmin[j] = std::max(0,        std::min(bboxmin[j], pts[i][j])); 
              bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j])); 
          } 
      } 
      Vec2i P; 
      for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) { 
          for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) { 
              Vec3f bc_screen  = barycentric(pts, P); 
              if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue; 
              image.set(P.x, P.y, color); 
          } 
      } 
  } 
   
  int main(int argc, char** argv) { 
      TGAImage frame(200, 200, TGAImage::RGB); 
      Vec2i pts[3] = {Vec2i(10,10), Vec2i(100, 30), Vec2i(190, 160)}; 
      triangle(pts, frame, TGAColor(255, 0, 0)); 
      frame.flip_vertically(); // to place the origin in the bottom left corner of the image 
      frame.write_tga_file("framebuffer.tga");
      return 0; 
  }
  ```

  - [关于点在三角形内的算法精度问题](https://blog.codingnow.com/2018/11/float_precision_problem.html#more)
  - barycentric()函数计算给定三角形和一个点P返回其重心坐标。上边已经详细介绍了。
  - 下边我们详细介绍triangle()函数。
    - 第一，它首先计算了给定三角形的边界框，由两个点确定（左下角，右上角）
    - 遍历整个边界框的像素点，如果在三角形内，则在image中设置该点。

<img src="E:\GameLearning\tinyRenderer\Notes\CH02\12.png" style="zoom:100%" div align="center"/>

#### Flat shading render

- 我们第一节讲解了如何画一个模型的线框。那么我们在本节的基础上，来完整的画出整个模型（填充随机颜色）。

  ```C++
  for (int i=0; i<model->nfaces(); i++) { 
      std::vector<int> face = model->face(i); 
      Vec2i screen_coords[3]; 
      for (int j=0; j<3; j++) { 
          Vec3f world_coords = model->vert(face[j]); 
          screen_coords[j] = Vec2i((world_coords.x+1.)*width/2., (world_coords.y+1.)*height/2.); 
      } 
      triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(rand()%255, rand()%255, rand()%255, 255)); 
  }
  ```

 - 代码很简单，我们会遍历所有的三角形，并将世界坐标转换到屏幕坐标中，依次画三角形。下一节，我们学习更多的坐标系。效果如下：
  <img src="E:\GameLearning\tinyRenderer\Notes\CH02\13.png" style="zoom:60%" div align="center"/>

- 我们尝试添加一些光照。注意：相同的光照强度，当物体表面和光线垂直时，光照强度最大。
  <img src="E:\GameLearning\tinyRenderer\Notes\CH02\14.jpg" style="zoom:60%" div align="center"/>
  <img src="E:\GameLearning\tinyRenderer\Notes\CH02\15.jpg" style="zoom:60%" div align="center"/>

- 如果平面和光源方向平行，那么几乎没有光照。光照强度等于光照向量和给定三角形的法向量的点积。三角形的法向量可以简单的由它两边相同的叉积求。（其实模型中提供了）

- 还有一点要注意，此课程仅仅展示颜色的线性计算。**但是**(128, 128, 128)的光照亮度并不是(256, 256, 256)的一半。我们就忽略gamma修正，包容我们光照计算的不正确。[Gamma修正参看参考](https://learnopengl.com/Advanced-Lighting/Gamma-Correction)

  ```c++
  for (int i=0; i<model->nfaces(); i++) { 
      std::vector<int> face = model->face(i); 
      Vec2i screen_coords[3]; 
      Vec3f world_coords[3]; 
      for (int j=0; j<3; j++) { 
          Vec3f v = model->vert(face[j]); 
          screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.); 
          world_coords[j]  = v; 
      } 
      Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]); 
      n.normalize(); 
      float intensity = n*light_dir; 
      if (intensity>0) { 
          triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity*255, intensity*255, intensity*255, 255)); 
      } 
  }
  ```

  - 注意，点积可能是负的。那这意味着什么？这意味着光照来自背面的多边形。If the scene is well modelled (it is usually the case), we can simply discard this triangle. This allows us to quickly remove some invisible triangles. It is called Back-face culling.
  <img src="E:\GameLearning\tinyRenderer\Notes\CH02\16.png" style="zoom:60%" div align="center"/>
  - 注意图片中的嘴唇部分。这是因为我们粗糙的裁剪手法（它仅仅对凸多边形管用）。当我们使用z-buffer时，我们将会改善这一情况，


