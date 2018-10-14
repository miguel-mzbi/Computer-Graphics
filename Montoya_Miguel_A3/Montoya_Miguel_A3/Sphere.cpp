#include <vector>
#include "Material.h"
#include "Sphere.h"

Sphere::Sphere() {}

Sphere::Sphere(float r, std::vector<float> &center, Material const &material) {
	this->r = r;
	this->center = center;
	this->material = material;
}
