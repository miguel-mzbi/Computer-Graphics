#pragma once

#ifndef MATERIAL_H
#define MATERIAL_H

#include "Color.h"

class Material {

	public:
		float shine;
		Color ambientC;
		Color diffuseC;
		Color specularC;

		Material();
		Material(float shine, Color const &ambientC, Color const &diffuseC, Color const &specularC);
};

#endif
