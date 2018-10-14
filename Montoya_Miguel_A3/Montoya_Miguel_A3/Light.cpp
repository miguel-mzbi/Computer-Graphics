#include <vector>
#include "Color.h"
#include "Light.h"

Light::Light() {}

Light::Light(std::vector<float> const &position, Color const &ambientC, Color const &diffuseC, Color const &specularC) {
	this->position = position;
	this->diffuseC = diffuseC;
	this->ambientC = ambientC;
	this->specularC = specularC;
}

