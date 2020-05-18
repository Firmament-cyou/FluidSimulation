#include "Fluid.h"
#include <algorithm>

using namespace std;

void Fluid::onUpdate(float delta_time)
{
	putInGrid();
	updateDensities();
	updatePressures();

	fill(particle_accelerations_.begin(), particle_accelerations_.end(), glm::vec3(0.0f, 0.0f, 0.0f));

	aOfGravity();
	aOfPressure();
	aOfViscosity();

	updateVelocities(delta_time);
	updatePositions(delta_time);

	for (int i = 0; i < total_; ++i) {
		int j = i * 3;
		draw_ptr_[j] = particle_positions_[i].x;
		draw_ptr_[j + 1] = particle_positions_[i].y;
		draw_ptr_[j + 2] = particle_positions_[i].z;
	}
}

void Fluid::reset()
{
	float d = 6.0f / (side_ - 1);
	int iter1 = 0;
	for (int i = 0; i < side_ / 2; ++i) {
		for (int j = 0; j < side_; ++j) {
			for (int k = 0; k < side_; ++k) {
				particle_positions_[iter1++] = glm::vec3(i * d, j * d - 3.0f, k * d - 3.0f);
			}
		}
	}
	fill(particle_velocities_.begin(), particle_velocities_.end(), glm::vec3(0.0f, 0.0f, 0.0f));
}

void Fluid::putInGrid()
{
	grid_.clear();
	for (int i = 0; i < total_; ++i) {
		glm::vec3 cur = particle_positions_[i];
		int x = (cur.x + 3.0f) / 0.2f;
		int y = (cur.y + 3.0f) / 0.2f;
		int z = (cur.z + 3.0f) / 0.2f;
		grid_[x * 900 + y * 30 + z].push_back(i);
	}
}

void Fluid::updateDensities()
{
	for (int i = 0; i < total_; ++i) {
		glm::vec3 cur = particle_positions_[i];
		int x = (cur.x + 3.0f) / 0.2f;
		int y = (cur.y + 3.0f) / 0.2f;
		int z = (cur.z + 3.0f) / 0.2f;
		particle_densities_[i] = 0.0f;
		for (int tmpx = x - 1; tmpx <= x + 1; ++tmpx) {
			for (int tmpy = y - 1; tmpy <= y + 1; ++tmpy) {
				for (int tmpz = z - 1; tmpz <= z + 1; ++tmpz) {
					int tmp_index = tmpx * 900 + tmpy * 30 + tmpz;
					if (grid_.count(tmp_index) == 0) continue;
					for (auto& near : grid_[tmp_index]) {
						float d = glm::distance(particle_positions_[i], particle_positions_[near]);
						int dd = d / delta_d_;
						if (dd >= 256) continue;
						particle_densities_[i] += kernels_->poly6_table[dd];
					}
				}
			}
		}
	}
}

void Fluid::updatePressures()
{
	for (int i = 0; i < total_; ++i) {
		particle_pressures_[i] = K_ * (particle_densities_[i] - den0_);
	}
}

void Fluid::aOfGravity()
{
	for (auto& vec3 : particle_accelerations_) {
		vec3.y -= g_;
	}
}

void Fluid::aOfPressure()
{
	for (int i = 0; i < total_; ++i) {
		glm::vec3 cur = particle_positions_[i];
		int x = (cur.x + 3.0f) / 0.2f;
		int y = (cur.y + 3.0f) / 0.2f;
		int z = (cur.z + 3.0f) / 0.2f;
		for (int tmpx = x - 1; tmpx <= x + 1; ++tmpx) {
			for (int tmpy = y - 1; tmpy <= y + 1; ++tmpy) {
				for (int tmpz = z - 1; tmpz <= z + 1; ++tmpz) {
					int tmp_index = tmpx * 900 + tmpy * 30 + tmpz;
					if (grid_.count(tmp_index) == 0) continue;
					for (auto& near : grid_[tmp_index]) {
						if (i == near) continue;
						float d = glm::distance(particle_positions_[i], particle_positions_[near]);
						int dd = d / delta_d_;
						if (dd >= 256) continue;
						glm::vec3 vec = (particle_pressures_[i] + particle_pressures_[near]) / 
							(2 * particle_densities_[i] * particle_densities_[near] * d) *
							(particle_positions_[i] - particle_positions_[near]);
						particle_accelerations_[i] += vec * kernels_->spiky_table[dd];
					}
				}
			}
		}
	}
}

void Fluid::aOfViscosity()
{
	for (int i = 0; i < total_; ++i) {
		glm::vec3 cur = particle_positions_[i];
		int x = (cur.x + 3.0f) / 0.2f;
		int y = (cur.y + 3.0f) / 0.2f;
		int z = (cur.z + 3.0f) / 0.2f;
		for (int tmpx = x - 1; tmpx <= x + 1; ++tmpx) {
			for (int tmpy = y - 1; tmpy <= y + 1; ++tmpy) {
				for (int tmpz = z - 1; tmpz <= z + 1; ++tmpz) {
					int tmp_index = tmpx * 900 + tmpy * 30 + tmpz;
					if (grid_.count(tmp_index) == 0) continue;
					for (auto& near : grid_[tmp_index]) {
						if (i == near) continue;
						float d = glm::distance(particle_positions_[i], particle_positions_[near]);
						int dd = d / delta_d_;
						if (dd >= 256) continue;
						glm::vec3 vec = mu_ * (particle_velocities_[near] - particle_velocities_[i]) /
							(particle_densities_[i] * particle_densities_[near]);
						particle_accelerations_[i] += vec * kernels_->viscosity_table[dd];
					}
				}
			}
		}
	}
}

void Fluid::updateVelocities(float delta_time)
{
	for (int i = 0; i < total_; ++i) {
		particle_velocities_[i] += particle_accelerations_[i] * delta_time;
	}
}

void Fluid::updatePositions(float delta_time)
{
	for (int i = 0; i < total_; ++i) {
		glm::vec3& pos = particle_positions_[i];
		glm::vec3& vel = particle_velocities_[i];
		pos += vel * delta_time;
		if (pos.x >  3.0f) { pos.x =  3.0f; vel.x = 0.0f; }
		if (pos.x < -3.0f) { pos.x = -3.0f; vel.x = 0.0f; }
		if (pos.y >  3.0f) { pos.y =  3.0f; vel.y = 0.0f; }
		if (pos.y < -3.0f) { pos.y = -3.0f; vel.y = 0.0f; }
		if (pos.z >  3.0f) { pos.z =  3.0f; vel.z = 0.0f; }
		if (pos.z < -3.0f) { pos.z = -3.0f; vel.z = 0.0f; }
	}
}
