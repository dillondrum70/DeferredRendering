#ifndef DEPTH_BUFFER_H
#define DEPTH_BUFFER_H

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "imgui.h"

class RenderBuffer
{
public:

	void Create(int screenWidth, int screenHeight);

	unsigned int GetRenderBuffer() { return rbo; }
	glm::vec2 GetDimensions() { return screenDimensions; }

private:

	unsigned int rbo;
	glm::vec2 screenDimensions;
};

#endif