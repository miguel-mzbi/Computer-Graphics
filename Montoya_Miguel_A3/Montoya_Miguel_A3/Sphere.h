#pragma once

#ifndef SPHERE_H
#define SPHERE_H

class Sphere {
	public:
		float r;
		std::vector<float> center;
		Material material;

		Sphere();
		Sphere(float r, std::vector<float> &center, Material const &material);
};

#endif