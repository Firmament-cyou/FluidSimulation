#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Kernels.hpp"

class Fluid
{
public:
	Fluid(int side, float* draw_ptr): side_(side), draw_ptr_(draw_ptr) {
		total_ = side_ * side_ * side_;
		particle_positions_.resize(total_);
		particle_velocities_.resize(total_);
		particle_accelerations_.resize(total_);
		particle_densities_.resize(total_);
		particle_pressures_.resize(total_);
		// 填充正方体的一半
		reset();
	}

	~Fluid() {}

	void onUpdate(float delta_time);
	void reset();

private:
	int side_, total_;
	float h_ = 0.3f, m_ = 1.0f; // 核半径，质点质量
	float g_ = 0.1f, K_ = 0.1f, mu_ = 0.00001f, den0_ = 0.0f; // 重力加速度，流体系数（温度相关），动力粘性系数，静态密度
	std::vector<glm::vec3> particle_positions_;
	std::vector<glm::vec3> particle_velocities_;
	std::vector<glm::vec3> particle_accelerations_;
	std::vector<float> particle_densities_;
	std::vector<float> particle_pressures_;
	// 30 * 30 * 30 的稀疏离散网格
	std::unordered_map<int, std::vector<int>> grid_;
	std::unique_ptr<Kernels> kernels_ = std::make_unique<Kernels>(h_, m_);
	float delta_d_ = h_ / 256;
	float* draw_ptr_;

	void putInGrid();
	void updateDensities();
	void updatePressures();

	// 加速度计算，调用前先清空 acceleration 数组
	void aOfGravity();
	void aOfPressure();
	void aOfViscosity();

	void updateVelocities(float delta_time);
	void updatePositions(float delta_time);
};

