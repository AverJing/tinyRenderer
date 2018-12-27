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
#include <cstdint>
#include "Random.h"

struct Color {
	uint8_t uintR;
	uint8_t uintG;
	uint8_t uintB;
	uint8_t uintA;

	Color(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255)
		:uintR(r), uintG(g), uintB(b), uintA(a) {}

	int GetR() const { return uintR; }
	int GetG() const { return uintG; }
	int GetB() const { return uintB; }
	int GetA() const { return uintA; }

	static Color RandomColor() {
		return Color(Random::ColorElem(), Random::ColorElem(), Random::ColorElem());
	}
};