#ifndef POSTPROCESS_EFFECT_H
#define POSTPROCESS_EFFECT_H

#include "../EW/Shader.h"

#include "imgui.h"

#include <vector>

class FramebufferObject;

class PostprocessEffect
{
public:

	PostprocessEffect(Shader* shader, std::string name);

	virtual void ExposeImGui();

	virtual void SetupShader(const std::vector<unsigned int>& colorBuffers);
	
	Shader* GetShader() { return _shader; }
	std::string GetName() { return _name; }

	virtual void SetParent(FramebufferObject* parent);

protected:

	Shader* _shader;
	std::string _name;

	FramebufferObject* _parent;

private:
};

#endif