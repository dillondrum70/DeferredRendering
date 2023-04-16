#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "glm/glm.hpp"

struct PointLight
{
	float intensity = 1.f;

	glm::vec3 color = glm::vec3(1);

	glm::vec3 pos = glm::vec3(0, 5, 0);

	void ExposeImGui(bool canMove);
};

#endif
