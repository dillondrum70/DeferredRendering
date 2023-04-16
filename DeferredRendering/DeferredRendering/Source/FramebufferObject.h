#ifndef FRAMEBUFFER_OBJECT_H
#define FRAMEBUFFER_OBJECT_H

#include <vector>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "imgui.h"

#include "Texture.h"
#include "RenderBuffer.h"

#include "Postprocessing/PostprocessEffect.h"
#include "Postprocessing/GrayscaleEffect.h"
#include "Postprocessing/InvertEffect.h"
#include "Postprocessing/EdgeDetectEffect.h"
#include "Postprocessing/ChromaticEffect.h"
#include "Postprocessing/BlurEffect.h"
#include "Postprocessing/BloomEffect.h"

class FramebufferObject
{
public:

	FramebufferObject();
	~FramebufferObject();

	unsigned int GetId() { return id; }
	glm::vec2 GetDimensions() { return dimensions; }
	bool IsComplete();

	void SetDimensions(glm::vec2 d) { dimensions = d; }

	void Create();
	void AddColorAttachment(Texture& buffer, GLenum attachmentNum);
	void AddColorAttachment(RenderBuffer& buffer, GLenum attachmentNum);
	void AddDepthAttachment(Texture& buffer);
	void AddDepthAttachment(RenderBuffer& buffer);
	void Bind();
	void Unbind(glm::vec2 viewportDimensions);
	void Clear(glm::vec3 clearColor);
	void Destroy();

	void ExposeImGui();

	void AddEffect(PostprocessEffect* effect);

	void SetupShader();

private:

	unsigned int id;
	glm::vec2 dimensions;

	std::vector<unsigned int> colorAttachments;
	std::vector<unsigned int> depthAttachments;

	int currentEffect = 0;
	std::vector<PostprocessEffect*> postprocessEffects;
};

#endif