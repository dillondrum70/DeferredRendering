#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "glm/glm.hpp"

struct SpotLight
{
	glm::vec3 pos = glm::vec3(0, 5, 0);
	glm::vec3 dir = glm::vec3(0, -1, 0);

	float intensity = 1.f;
	glm::vec3 color = glm::vec3(1);

	float range = 10;
	float innerAngle = 20;
	float outerAngle = 30;
	float angleFalloff = 2;

	void ExposeImGui(bool manuallyMove);
};

#endif
