/*
* 此处进行的数据处理没有实际的物理意义
* 全部是以优化性能为主，代码并不优雅，甚至可能不知所云
* 日后有空再重写处理吧（没空就不改了233）
**/
#pragma once

#include <array>
#include <cmath>

class Kernels {
public:
	Kernels(float h, float m) : h_(h), m_(m) {
		delta_d_ = h_ / 256;
		init_poly6();
		init_spiky();
		init_viscosity();
	}

	std::array<float, 256> poly6_table;
	std::array<float, 256> spiky_table;
	std::array<float, 256> viscosity_table;

private:
	const float PI = 3.1415926536f;
	float h_, m_, delta_d_;

	void init_poly6() {
		float d = 0.0f;
		const float c = (m_ * 365) / (64 * PI * pow(h_, 9));
		for (int i = 0; i < 256; ++i) {
			poly6_table[i] = c * pow((h_ * h_ - d * d), 3);
			d += delta_d_;
		}
	}

	void init_spiky() {
		float d = 0.0f;
		const float c = (m_ * 45) / (PI * pow(h_, 6));
		for (int i = 0; i < 256; ++i) {
			spiky_table[i] = c * pow((h_ - d), 2);
			d += delta_d_;
		}
	}

	void init_viscosity() {
		float d = 0.0f;
		const float c = (m_ * 45) / (PI * pow(h_, 6));
		for (int i = 0; i < 256; ++i) {
			viscosity_table[i] = c * (h_ - d);
			d += delta_d_;
		}
	}
};
