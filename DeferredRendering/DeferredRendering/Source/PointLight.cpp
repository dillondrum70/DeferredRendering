#include "PointLight.h"

#include "imgui.h"

void PointLight::ExposeImGui(bool canMove)
{
	if (canMove)
	{
		ImGui::DragFloat3("Light Position", &pos.x);
	}

	ImGui::SliderFloat("Light Intensity", &intensity, 0.f, 1.f);

	ImGui::ColorEdit3("Light Color", &color.x);
}