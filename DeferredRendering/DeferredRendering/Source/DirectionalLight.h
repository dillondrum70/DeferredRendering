#ifndef DIRECTIONALLIGHT_H
#define DIRECTIONALLIGHT_H

#include "glm/glm.hpp"

struct DirectionalLight
{
	float intensity = 1.f;

	glm::vec3 color = glm::vec3(1);

	glm::vec3 dir = glm::vec3(-10, -10, -10);

	void ExposeImGui(bool manuallyMove);
};

#endif
