#include "Color.h"
#include "Material.h"

Material::Material() {}

Material::Material(float shine, Color const &ambientC, Color const &diffuseC, Color const &specularC) {
	this->shine = shine;
	this->ambientC = ambientC;
	this->diffuseC = diffuseC;
	this->specularC = specularC;
}

