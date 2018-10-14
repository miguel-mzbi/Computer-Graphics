#pragma once

#ifndef LIGHT_H
#define LIGHT_H

#include "Color.h"

class Light {

	public:
		std::vector<float> position;
		Color ambientC;
		Color diffuseC;
		Color specularC;

		Light();
		Light(std::vector<float> const &position, Color const &ambientC, Color const &diffuseC, Color const &specularC);
};

#endif
