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

#include <random>
#include <ctime>
#include <chrono>

struct Random {
	//注意，如果随机引擎不是静态的，每次调用都是一样的。
	//或者可以置种子，但是一般使用time函数不够精确
	static uint8_t ColorElem() {
		static std::default_random_engine e;
		static std::uniform_int_distribution<int> t(0, 255); //[0, 255]

		return t(e);
	}
	
	static float Value() {//[0,1]
		static std::default_random_engine e;
		static std::uniform_real_distribution<float> t(0, 1);

		return t(e);
	}
	/*
	static float Value() {//[0,1]
		//还是不行
		//time_t不够精确
		std::default_random_engine e(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
		std::uniform_real_distribution<float> t(0, 1);

		return t(e);
	}*/

	static float Value2()
	{
		return (double)rand() / (RAND_MAX);
	}	
};

