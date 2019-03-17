# tinyRenderer

- 基于ssloy[教程](https://github.com/ssloy/tinyrenderer)，其教程练习分别对应相关main函数。Note文件夹中保存我个人对教程的粗略翻译和自己简单的理解。
- [软渲染器项目](https://github.com/AverJing/tinyRenderer/tree/master/tinyRenderer/tinySoftRenderer)
- 已完成：
  - 借助SDL创建窗口，键盘监听，以及图像绘制。
  - Bresenham画线算法，Cohen–Sutherland裁剪算法，基于重心坐标的三角形光栅化。
  - 坐标(World/View/Projection/Viewport)变换，支持obj文件的加载，通过深度缓存判断图像前后，支持纹理贴图，实现Phone着色模型。
- 目前的计划：
  - 使用SDL库，添加鼠标，键盘响应，控制摄像机，可以从不同角度观察物体。