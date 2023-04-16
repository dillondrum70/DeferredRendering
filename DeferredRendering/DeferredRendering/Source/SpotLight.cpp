#include "SpotLight.h"

#include "imgui.h"

void SpotLight::ExposeImGui(bool manuallyMove)
{
	if (manuallyMove)
	{
		ImGui::DragFloat3("Spotlight Position", &pos.x);
		ImGui::DragFloat3("Spotlight Direction", &dir.x);
	}

	ImGui::SliderFloat("Spotlight Intensity", &intensity, 0.f, 1.f);
	ImGui::ColorEdit3("Spotlight Color", &color.x);

	ImGui::SliderFloat("Spotlight Range", &range, 1.f, 30.f);
	ImGui::SliderFloat("Spotlight Inner Angle", &innerAngle, 10.f, 60.f);
	ImGui::SliderFloat("Spotlight Outer Angle", &outerAngle, 10.f, 60.f);
	ImGui::SliderFloat("Spotlight Angle Falloff", &angleFalloff, 0.f, 6.f);
}