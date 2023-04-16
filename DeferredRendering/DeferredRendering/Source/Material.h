#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>

#include "imgui.h"

struct Material
{
	glm::vec3 color = glm::vec3(1);
	float ambientK = .15f;
	float diffuseK = .5f;
	float specularK = .25f;
	float shininess = 2;
	float normalIntensity = 1;

	void ExposeImGui();
};

#endif
