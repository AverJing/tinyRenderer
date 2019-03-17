#pragma once

#include "geometry.h"
#include "Color.h"

struct IShader {
	virtual ~IShader() {};
	virtual Vec4f vertex(int iface, int nthervt) = 0;
	virtual bool fragment(Vec3f bar, Color& color) = 0;
};