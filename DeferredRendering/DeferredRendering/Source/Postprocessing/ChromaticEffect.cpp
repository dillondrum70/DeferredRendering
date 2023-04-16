#include "ChromaticEffect.h"

#include "GLFW/glfw3.h"

void ChromaticEffect::ExposeImGui()
{
	PostprocessEffect::ExposeImGui();

	ImGui::SliderFloat("Aberration Amount", &_aberrationAmount, -.1f, .1f);
	ImGui::SliderFloat("Flicker Speed", &_flickerSpeed, 0, 100);
	ImGui::SliderFloat("Flicker Amount", &_flickerAmount, 0, 1);
}


void ChromaticEffect::SetupShader(const std::vector<unsigned int>& colorBuffers)
{
	PostprocessEffect::SetupShader(colorBuffers);

	_shader->setFloat("_AberrationAmount", _aberrationAmount);
	_shader->setFloat("_Time", (float)glfwGetTime());
	_shader->setFloat("_FlickerSpeed", _flickerSpeed);
	_shader->setFloat("_FlickerAmount", _flickerAmount);
}