#include "RenderBuffer.h"

void RenderBuffer::Create(int screenWidth, int screenHeight)
{
	screenDimensions = glm::vec2(screenWidth, screenHeight);

	glGenRenderbuffers(1, &rbo);

	glBindRenderbuffer(GL_RENDERBUFFER, rbo);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, screenWidth, screenHeight);
}