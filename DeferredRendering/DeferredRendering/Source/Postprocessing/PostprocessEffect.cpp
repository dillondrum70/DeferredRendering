#include "PostprocessEffect.h"

#include "FramebufferObject.h"

PostprocessEffect::PostprocessEffect(Shader* shader, std::string name)
{
	_shader = shader;
	_name = name;
}


void PostprocessEffect::ExposeImGui()
{
	ImGui::Text(_name.c_str());
}

void PostprocessEffect::SetupShader(const std::vector<unsigned int>& colorBuffers)
{
	if (colorBuffers.size() >= 1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	
	if (colorBuffers.size() >= 2)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, colorBuffers[1]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	_shader->use();
	_shader->setInt("_ColorTex", 0);
}

void PostprocessEffect::SetParent(FramebufferObject* parent)
{
	_parent = parent;
}