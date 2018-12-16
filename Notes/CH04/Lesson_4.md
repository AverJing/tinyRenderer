[Lesson 4](https://github.com/ssloy/tinyrenderer/wiki/Lesson-4:-Perspective-projection)：Perspective projection（z-buffer）

### 目标

- 上一节我们以正射投影方式渲染模型，这一节我们将学习使用透视投影渲染。
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\01.png" style="zoom:60%" div align="center">

### 2D几何
------------------------
#### 线性转化（列主序）
- 平面的线性变化可以使用矩阵。如果我对一个点(x, y)施加变化，如下
  <img src="E:\GameLearning\tinyRenderer\Notes\CH04\02.png" style="zoom:100%" div align="center">

- 同等变化（不改变该点）
  <img src="E:\GameLearning\tinyRenderer\Notes\CH04\03.png" style="zoom:100%" div align="center">

- 放缩变换
  <img src="E:\GameLearning\tinyRenderer\Notes\CH04\04.png" style="zoom:100%" div align="center">

- 将上述变化（放缩）对白色物体施加后就是黄色的物体。红线和绿线各自代表x，y轴单位线段。
  <img src="E:\GameLearning\tinyRenderer\Notes\CH04\05.png" style="zoom:100%" div align="center">

- 我们为什么要使用矩阵？因为它很方便。Well, the true reason hides here: very, very often we wish to transform our object with many transformations in a row.假设你写的变化函数如下

  ```C++
  vec2 foo(vec2 p) return vec2(ax+by, cx+dy);
  vec2 bar(vec2 p) return vec2(ex+fy, gx+hy);
  [..]
  for (each p in object) {
      p = foo(bar(p));
  }
  ```
- 代码表示我们对模型的每个顶点进行两次线性变换。矩阵可以将多个变换整合在一起。牛逼。
- 好了，我们继续。对角线矩阵可以对物体进行放缩。那么斜对角线呢？看一下下边的变换
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\07.png" style="zoom:100%" div align="center">
- 下边的图就是结果
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\08.png" style="zoom:100%" div align="center">
- 图片沿着x轴方向削尖。It is a simple shearing along the x-axis. Another anti-diagonal element shears our space along the y-axis. Thus, there are two base linear transformations on a plane: scaling and shearing. Many readers react: wait, what about rotations?!
- It turns out that any rotation (around the origin) can be represented as a composite action of three shears, here the white object is transformed to the red one, then to the green one and finally to the blue:
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\09.png" style="zoom:100%" div align="center">
- 旋转矩阵如下：
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\10.png" style="zoom:100%" div align="center">
- 但是请记住，矩阵**不具有交换律**。
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\11.png" style="zoom:100%" div align="center">
- 消减再旋转和旋转再消减不一样
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\12.png" style="zoom:100%" div align="center">

#### 2D仿射变换
- 2D线性变化都是由缩放和切变合成的。这意味着我们可以做任何线性变换。Those possibilities are great, but if we can not perform simple translations, our life will be miserable. Can we? Okay, translations are not linear, no problem, let us try to append translations after performing the linear part:
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\13.png" style="zoom:100%" div align="center">
- 结果很酷，我们可以旋转，缩放，shear和位移。而我们真正感兴趣的是将多种变换组合到一起。下边展示的就是两种变换组合起来的样子
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\14.png" style="zoom:100%" div align="center">

#### 齐次坐标
- 非常神奇。增一维解决问题。我增加了一列和一行，是原矩阵变成3*3的，并将向量的第三位一直设为1
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\15.png" style="zoom:100%" div align="center">
- 虽然增加了一维，但是并不影响实际的结果。神奇。
- 事实上，这个想法也非常简单。平行变换在2D空间并不是线性的。所以我们就将2D的变成3D的（简单的添加第三个分量）。这意味着我们的2D空间其实是平面z=1的平面（在3D中）。之后我们施加线性3D变换并将结果投影到我们的2D平面上。Parallel translations have not become linear, but the pipeline is simple. 我们如何将3D投射到2D上？简单的把x，y除以z即可。
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\16.png" style="zoom:100%" div align="center">

#### 等一下，千万不能除以0
- 我们回忆一下pipeline
  -  我们首先把2D提升到3D，将z分量设置为1
  -  在3D中做变换
  -  然后将每一个顶点从3D投射到2D中，For every point we want to project from 3D into 2D we draw a straight line between the origin and the point to project and then we find its intersection with the plane z=1.（就是将(x,y,z)变成(x/z, y/z， 1)）
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\17.png" style="zoom:100%" div align="center">
- 一个和平面z=1的向量投影到该平面的坐标？
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\18.png" style="zoom:100%" div align="center">
- Now let us descend on the rail, for example, the point (x,y,1/2) is projected onto (2x, 2y):
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\19.png" style="zoom:100%" div align="center">
- Let us continue, point (x,y,1/4) becomes (4x, 4y):
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\20.png" style="zoom:100%" div align="center">
- 如果z趋向于0，那么该投影点趋向于无穷大。换句话说，点 (x,y,0)应该投影到方向(x,y)无穷大，这其实就是向量。
- 齐次坐标可以用来区分向量和顶点。向量+向量=向量，向量-向量=向量，点+向量=点。

#### 复合变换
- 我之前谈到过，我们需要一沓变换的组合。为什么？我们先试着把一个物体绕着一个点旋转。我们可以从别的地方找到相关旋转矩阵，或者推导一下。
- We know to rotate around the origin, we know how to translate. It is all we need: translate (x0,y0) into the origin, rotate, un-translate, done:
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\21.png" style="zoom:100%" div align="center">
- In 3D sequences of actions will be a bit longer, but the idea is the same: we need to know few basic transformations and with their aid we can represent any composed action.

#### 和前边2D图形的分析一样
- 将正方形应用下边的变换
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\22.png" style="zoom:100%" div align="center">
- 原始物体是白色的，x轴是红色方向，y轴是绿色方向
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\23.png" style="zoom:60%" div align="center">
- 变换后的物体：
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\24.png" style="zoom:100%" div align="center">
- 这是一种神奇的变换。还记得之前讲的y-buffer吗？我将继续做同样的事情，我们将2D物体投影到垂线x=0上。在增加些要求，我们使用central projection，摄像机在点(5,0)的位置，指向原点。为了投影我们需要把顶点和摄影机连线（黄色的就是连线）。
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\25.png" style="zoom:100%" div align="center">
- 现在看一下变换后的样子，注意黄色的线还是之前的。
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\26.png" style="zoom:100%" div align="center">
- 如果我们使用正射投影将红色物体投影到屏幕上，我们会找到完全相同的点。仔细看一下变换如何起作用的，顶点都发生了变化（废话），**但是那些靠近摄像机的顶点拉伸，远离摄像机的顶点缩张**。如果我们选择合适的系数（本例选的是-1/5），那么就可以得到投影变换。

#### 完全的3D
- 解释下上边的神奇变换。对于3D的仿射（affine）变换，我们将采用**齐次坐标**即(x,y,z) 扩展为(x,y,z,1)，之后我们再将4D的投影到3D中。例如，我们可以采用这样的变换：
 <img src="E:\GameLearning\tinyRenderer\Notes\CH04\27.png" style="zoom:100%" div align="center">
 - The retro-projection gives us the following 3D coordinages
 <img src="E:\GameLearning\tinyRenderer\Notes\CH04\28.png" style="zoom:100%" div align="center">
 - 记住上边的结果，先暂时放一边。我们先考虑标准的透视投影的定义（没有4D变换参与）。我们要将点P(x, y, z)投影到平面z=0上，摄像机是在z轴上，坐标是(0, 0, c)
 <img src="E:\GameLearning\tinyRenderer\Notes\CH04\29.png" style="zoom:100%" div align="center">
 - 三角形ABC和三角形ODC是相似的。继而得到 |AB|/|AC|=|OD|/|OC|即x/(c-z) = x'/c
    <img src="E:\GameLearning\tinyRenderer\Notes\CH04\30.png" style="zoom:100%" div align="center">
  - 同理可以得到
      <img src="E:\GameLearning\tinyRenderer\Notes\CH04\31.png" style="zoom:100%" div align="center">
   - 结合上边的公式，你会发现r=-1/c

#### 总结
- So, if we want to compute a central projection with a camera (important!) camera **located on the z-axis** with distance c from the origin, then we embed the point into 4D by augmenting it with 1, then we multiply it with the following matrix, and retro-project it into 3D.
<img src="E:\GameLearning\tinyRenderer\Notes\CH04\32.png" style="zoom:100%" div align="center">
- We deformed our object in a way, that simply forgetting its z-coordinate we will get a drawing in a perspective. If we want to use the z-buffer, then, naturally, do not forget the z. The code is available here, its result is visible in the very beginning of the article.