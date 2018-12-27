/*
*
*
*@author: Aver Jing
*@description：
*@date：
*
*
*/
#pragma once

struct RenderContext {
	int width;  //窗口宽度
	int height;  //窗口高度
	int bpp;	//像素字节数， RGBA
	int* backBuffer;	//像素首地址
	float* depthBuffer;//深度值首地址

	~RenderContext() {
		delete backBuffer;
		delete depthBuffer;
	}
};
