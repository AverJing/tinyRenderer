### [Lesson 1](https://github.com/ssloy/tinyrenderer/wiki/Lesson-1:-Bresenham%E2%80%99s-Line-Drawing-Algorithm)：Bresenham划线法算法



#### 第一次尝试

- 第一节课的目标是网格。为了实现目标，我们必须先掌握如何画线。考虑为什么Bresenham算法这么好，我们逐步尝试后，你会拍手叫好。

- ```c++
  void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
      for (float t=0.; t<1.; t+=.01) { 
          int x = x0*(1.-t) + x1*t; 
          int y = y0*(1.-t) + y1*t; 
          image.set(x, y, color); 
      } 
  }
  //简单的两点间插值计算
  //虽然简单但是插值用的非常广泛
  ```
 <img src="E:\GameLearning\tinyRenderer\Notes\CH01\01.png" style="zoom:100%" div align="center">

#### 第二次尝试

- 上边代码的问题在于常量的选择上（效率也很差），t=0.01，0.1等等，如果t过大线变成了点（虽然本质上都是些离散的点）
  <img src="E:\GameLearning\tinyRenderer\Notes\CH01\02.png" style="zoom:100%" div align="center"/>

- 我们很自然的考虑到画一个个像素点不就好了。下边给出示意代码（存在错误）

  ```c++
  void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
      for (int x=x0; x<=x1; x++) { 
          float t = (x-x0)/(float)(x1-x0); 
          int y = y0*(1.-t) + y1*t; 
          image.set(x, y, color); 
      } 
  }
  ```

  - 小心！``` (x-x0)/(float)(x1-x0); ``` 会不会出错？ 肯定会的。

- 我们尝试执行下边的代码

  ```c++
  line(13, 20, 80, 40, image, white); 
  line(20, 13, 40, 80, image, red); 
  line(80, 40, 13, 20, image, red);
  ```
<img src="E:\GameLearning\tinyRenderer\Notes\CH01\03.png" style="zoom:100%" div align="center"/>

- 其中白色的效果比较好，但是红色的两点之间间隙较大。并没有第三条线，因为第一条和第三条是一样的（但是两条线不会完全重合）。

#### 第三次尝试

- 我们可以通过交换两点坐标确保x坐标递增。注意上边的红线存在洞，因为该直线高度大于宽度。

- ```c++
  void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
      bool steep = false; 
      if (std::abs(x0-x1)<std::abs(y0-y1)) { // if the line is steep, we transpose the image 
          std::swap(x0, y0); 
          std::swap(x1, y1); 
          steep = true; 
      } 
      if (x0>x1) { // make it left−to−right 
          std::swap(x0, x1); 
          std::swap(y0, y1); 
      } 
      for (int x=x0; x<=x1; x++) { 
          float t = (x-x0)/(float)(x1-x0); 
          int y = y0*(1.-t) + y1*t; 
          if (steep) { 
              //这一步需要特别注意
              image.set(y, x, color); // if transposed, de−transpose 
          } else { 
              image.set(x, y, color); 
          } 
      } 
  }
  ```
<img src="E:\GameLearning\tinyRenderer\Notes\CH01\04.png" style="zoom:100%" div align="center"/>

#### 统计时间：第四次尝试

- 上述的代码还是不够高效，因为有除法，但是可读性很好。 注意上述代码没有assert也没有检查边界，差评。我们尝试对它进行优化，但是注意优化是很危险的事情。**特别注意**，对GPU和CPU的优化完全不是一回事。

- 我们应该注意到**每次都有相同的除数**，那么我们可以考虑把它移除循环。此处引入了**误差变量**，非常重要的优化。误差变量是实际直线的斜率，通过误差变量我们下一个像素点。

  ```C++
  void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
      bool steep = false; 
      if (std::abs(x0-x1)<std::abs(y0-y1)) { 
          std::swap(x0, y0); 
          std::swap(x1, y1); 
          steep = true; 
      } 
      if (x0>x1) { 
          std::swap(x0, x1); 
          std::swap(y0, y1); 
      } 
      int dx = x1-x0; 
      int dy = y1-y0; 
      float derror = std::abs(dy/float(dx)); 
      float error = 0; 
      int y = y0; 
      for (int x=x0; x<=x1; x++) { 
          if (steep) { 
              image.set(y, x, color); 
          } else { 
              image.set(x, y, color); 
          } 
          error += derror; 
          if (error>.5) { 
              y += (y1>y0?1:-1); 
              error -= 1.; 
          } 
      } 
  } 
  ```

#### 第五次尝试

- 我们为什么需要浮点数？唯一的原因就是我们需要除以dx以及和0.5比较。通过代替与原来的误差变量，我们可以摆脱浮点数。

  ```c++
  void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
      bool steep = false; 
      if (std::abs(x0-x1)<std::abs(y0-y1)) { 
          std::swap(x0, y0); 
          std::swap(x1, y1); 
          steep = true; 
      } 
      if (x0>x1) { 
          std::swap(x0, x1); 
          std::swap(y0, y1); 
      } 
      int dx = x1-x0; 
      int dy = y1-y0; 
      int derror2 = std::abs(dy)*2; 
      int error2 = 0; 
      int y = y0; 
      for (int x=x0; x<=x1; x++) { 
          if (steep) { 
              image.set(y, x, color); 
          } else { 
              image.set(x, y, color); 
          } 
          error2 += derror2; 
          if (error2 > dx) { 
              y += (y1>y0?1:-1); 
              error2 -= dx*2; 
          } 
      } 
  } 
  ```
  - 可以通过传递引用来避免拷贝。

#### 边框渲染

- 我们打算渲染一个模型的边框。模型格式采取[obj](https://en.wikipedia.org/wiki/Wavefront_.obj_file)的，将结果存放在图片中。

- 注意obj文件中的（v 顶点坐标  vt纹理坐标 vn平面法向量，更多部分参考wiki ）

- ```c++
  for (int i=0; i<model->nfaces(); i++) { 
      std::vector<int> face = model->face(i); 
      for (int j=0; j<3; j++) { 
          Vec3f v0 = model->vert(face[j]); 
          Vec3f v1 = model->vert(face[(j+1)%3]); 
          int x0 = (v0.x+1.)*width/2.; 
          int y0 = (v0.y+1.)*height/2.; 
          int x1 = (v1.x+1.)*width/2.; 
          int y1 = (v1.y+1.)*height/2.; 
          line(x0, y0, x1, y1, image, white); 
      } 
  }
  ```
  <img src="E:\GameLearning\tinyRenderer\Notes\CH01\05.png" style="zoom:100%" div align="center"/>

- 此部分对应Lesson1_main.cpp