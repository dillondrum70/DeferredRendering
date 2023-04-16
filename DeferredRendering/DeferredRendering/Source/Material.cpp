#include "Material.h"

void Material::ExposeImGui()
{
	ImGui::SetNextWindowSize(ImVec2(0, 0));	//Size to fit content
	ImGui::Begin("Material");

	ImGui::ColorEdit3("Material Color", &color.x);

	ImGui::SliderFloat("Material Ambient K", &ambientK, 0, 1);
	ImGui::SliderFloat("Material Diffuse K", &diffuseK, 0, 1);
	ImGui::SliderFloat("Material Specular K", &specularK, 0, 1);
	ImGui::SliderFloat("Material Shininess", &shininess, 1, 512);
	ImGui::SliderFloat("Normal Intensity", &normalIntensity, 0, 6);

	ImGui::End();
}