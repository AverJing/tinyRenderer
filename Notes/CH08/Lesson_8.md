## Lesson 8: Ambient occlusion 环境光散射（摘要）

[名词百科](https://baike.baidu.com/item/Ambient%20Occlusion/6216032?fr=aladdin)



- Phone反射模型（Ambient + Diffuse + Specular），LearnOpenGL上边光照部分也有对应的讲解
- 如图 ![01](E:\GameLearning\tinyRenderer\Notes\CH08\01.png)
- 一般ambient密度是常量（为什么）[wiki](https://en.wikipedia.org/wiki/Shading#Ambient_lighting)
  - 我觉得应该是物体的材质都是固定的，故而自身的环境光散射的参数是固定的

#### 全局光照第二次尝试：环境光散射

- 上一节计算**阴影贴图**用到了全局光照，**阴影贴图**可以提高渲染质量。
- 现在我们尝试使用Phone反射模型来进一步提高效果。此部分暂时仅仅使用ambient
- 如图                                          <img src="E:\GameLearning\tinyRenderer\Notes\CH08\02.png" style="zoom:50%" div align=center />
- 注意上边的图的Ambient各个部位密度不一样。
- 如下几个问题：
  - 当把环境光密度设置为常量时，光照会被等倍的向四周反射。
  -  Of course, it was made back in the old days where computing power was severely limited.（shader部分通过幂次的来计算reflect，不确定描述的意思。 过去算力有限不能够达到如此地步。）
  - 回忆计算**阴影贴图**时，我们分为两步，这让会使得FPS减一倍。

#### Brute force尝试

- 假定一个半球包围着我们要渲染的物体，在半球上的光线均匀向下散射（想想我们的天空）。

- 在半球上随机取1000个点，上千次的渲染物体，计算出模型哪些部分是可见的。

- 问题1：如何在半球上随机且均匀的取点？

  - <img src="E:\GameLearning\tinyRenderer\Notes\CH08\03.png" style="zoom:50%" div align=center />
  - [答案](http://mathworld.wolfram.com/SpherePointPicking.html)
  - 注意在球坐标如果直接 $$ \theta \in \lbrack 0, 2\pi )  $$ 和$$\phi \in [0,\pi]$$ 随机取，更多的点会趋向轴，因为面积元素$$d\Omega = sin\phi d\theta d\phi$$是$$\phi$$的函数
  - 正确做法是 $$d\Omega =  -d\theta d(cos\phi) $$  $$\theta = 2\pi u$$  $$ \phi = \arccos(2v-1)$$ 令u和v随机即可得结果
  - ```c++
    Vec3f rand_point_on_unit_sphere() {
    float u = (float)rand() / (float)RAND_MAX;
    float v = (float)rand() / (float)RAND_MAX;
    float theta = 2.f*M_PI*u;
    float phi = acos(2.f*v - 1.f);
    return Vec3f(sin(phi)*cos(theta), sin(phi)*sin(theta), cos(phi));
    }```
    ```

- 问题2：我们把可见信息存在哪里？

  - 可以把结果存在纹理中

  - 因此，我们可以每次从半球中取一个点分两步渲染，下边是第一个shader和渲染图片

  - ```c++
    virtual bool fragment(Vec3f gl_FragCoord, Vec3f bar, TGAColor &color) {
            color = TGAColor(255, 255, 255)*((gl_FragCoord.z+1.f)/2.f);
            return false;
        }
    ```

  - <img src="E:\GameLearning\tinyRenderer\Notes\CH08\04.png" style="zoom:50%" div align=center />

  - 图片看起来不生动，这个图片告诉我们的是每个位置的z-buffer(越大越亮)，忘记了看前边的教程，**非常重要**。接下来我们做另一个步骤

  - ```c++
    virtual bool fragment(Vec3f gl_FragCoord, Vec3f bar, TGAColor &color) {
            Vec2f uv = varying_uv*bar;
            if (std::abs(shadowbuffer[int(gl_FragCoord.x+gl_FragCoord.y*width)]-gl_FragCoord.z)<1e-2) {
                occl.set(uv.x*1024, uv.y*1024, TGAColor(255));
            }
            color = TGAColor(255, 0, 0);
            return false;
        }
    ```

  - ```occl.set(uv.x*1024, uv.y*1024, TGAColor(255));``` 

    - 这一行告诉我们如果段可见，将该纹理位置设置为白色（uv是纹理坐标）。下边的图就是从半球中选择一个点来计算的occl图像

    - <img src="E:\GameLearning\tinyRenderer\Notes\CH08\05.png" style="zoom:50%" div align=center />

    - 问题3： 为什么在可见三角形周边有如此多的洞？

      - 个人想法，因为某个可见点旁边可能存在许多不可见的点

    - 问题4：为什么有些三角形密度比其他区域密度大很多？

      - 个人想法，感觉还是和可见点（光照位置）有关，模型本身？

    - 我们重复上述过程1000次，把所有的occl图像平均后的结果如下

    - <img src="E:\GameLearning\tinyRenderer\Notes\CH08\06.png" style="zoom:50%" div align=center />

    - 很酷（学图形学，过程虽苦，结果真美，虽然这个没有添加颜色）。接下来让我们在没有光照消耗的情况下，简单的把上边得到的纹理贴到模型上去。

    - ```C++
      virtual bool fragment(Vec3f gl_FragCoord, Vec3f bar, TGAColor &color) {
              Vec2f uv = varying_uv*bar;
              int t = aoimage.get(uv.x*1024, uv.y*1024)[0];
              color = TGAColor(t, t, t);
              return false;
          }
      ```

    - aoimage就是上边重复1000次计算得到的纹理，渲染结果如下（我没找到这个模型文件）

    - <img src="E:\GameLearning\tinyRenderer\Notes\CH08\07.png" style="zoom:50%" div align=center />

    - 问题5：为什么它不开心（脸部偏暗）？

    - 和问题4有关。你注意到Diablo的纹理只有一个手臂吗？

      - 可以节省资源，两个手臂有相同的纹理坐标

    ##### 总结

    - 这种方法可以帮助我们重复计算静态几何体的环境光散射。计算时间消耗取决于你采样的次数，但实际上它并不重要，因为一旦你计算完成，我们可以重复利用。这个方法的**优点**是它的灵活性，可以用来计算**更复杂的光照**而不仅仅限制于球体。（不太懂）**缺点**是双倍纹理坐标计算消耗是不正确的，我们通过放置scotch tape来修复它（Disadvantage - for doubled texture coordinates the computation is not correct, we need to put some scotch tape to repair it (see the teaser image for this lesson).）想起来的在
    - [看练习部分](https://learnopengl-cn.github.io/02%20Lighting/04%20Lighting%20maps/)，是不是放射光贴图，可以增加局部效果，如灯带，眼睛发光

#### 屏幕空间的环境光散射

- 全局光照非常耗费资源，它需要多次的可见计算。我们试图寻找在计算时间和渲染质量的平衡点。下边的图片是我想要渲染的（回忆这节课程，我只是想要使用ambient occlusion）

- <img src="E:\GameLearning\tinyRenderer\Notes\CH08\08.png" style="zoom:50%" div align=center />

- 这是计算图像的shader

  ```c++
  struct ZShader : public IShader {
      mat<4,3,float> varying_tri;
  
      virtual Vec4f vertex(int iface, int nthvert) {
          Vec4f gl_Vertex = Projection*ModelView*embed<4>(model->vert(iface, nthvert));
          varying_tri.set_col(nthvert, gl_Vertex);
          return gl_Vertex;
      }
  
      virtual bool fragment(Vec3f gl_FragCoord, Vec3f bar, TGAColor &color) {
          color = TGAColor(0, 0, 0);
          return false;
      }
  };
  //注意 TGAColor(0, 0, 0);？？？
  //正确的，此时我仅仅关注z-buffer，图片还要经过后期处理。
  ```

- 下边是完整的“空”shader调用和后期处理过程

  ```C++
  ZShader zshader;
      for (int i=0; i<model->nfaces(); i++) {
          for (int j=0; j<3; j++) {
              zshader.vertex(i, j);
          }
          triangle(zshader.varying_tri, zshader, frame, zbuffer);
      }
  
      for (int x=0; x<width; x++) {
          for (int y=0; y<height; y++) {
              if (zbuffer[x+y*width] < -1e5) continue;
              float total = 0;
              for (float a=0; a<M_PI*2-1e-4; a += M_PI/4) {
                  total += M_PI/2 - max_elevation_angle(zbuffer, Vec2f(x, y), Vec2f(cos(a), sin(a)));
              }
              total /= (M_PI/2)*8;
              total = pow(total, 100.f);
              frame.set(x, y, TGAColor(total*255, total*255, total*255));
          }
      }
  /*
  “空”shader计算z-buffer。
  后期处理：图像中的每一个pixel，我向该pixel四周发射一些光线（此处是8个）。z-buffer是一个高度贴图，可以把它当成地形图（等高线？）。我要去计算8个方向的斜率
  函数 max_elevation_angle 给出我们抛出光线的最大斜率
  如果8条光线没有提升，那么这个pixel有较好的光照效果，附近地面是平整的。如果角度接近90°，那么当前pixel隐藏在某个峡谷下边，反射很少量的光
  In theory we need to compute the [solid angle](https://en.wikipedia.org/wiki/Solid_angle) for each point of the z-buffer, but we approximate it as a sum of (90°-max_elevation_angle) / 8. The pow(, 100.) is simply there to increase the contrast of the image.
  */
  ```

  - 下边就是只使用ambient occlusion渲染结果
  - <img src="E:\GameLearning\tinyRenderer\Notes\CH08\09.png" style="zoom:50%" div align=center />
  - 欣赏把。
  - <img src="E:\GameLearning\tinyRenderer\Notes\CH08\10.png" style="zoom:50%" div align=center />

