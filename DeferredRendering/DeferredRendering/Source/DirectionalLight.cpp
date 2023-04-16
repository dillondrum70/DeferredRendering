#include "DirectionalLight.h"

#include "imgui.h"

void DirectionalLight::ExposeImGui(bool manuallyMove)
{
	if (manuallyMove)
	{
		ImGui::DragFloat3("Light Direction", &dir.x);
	}

	ImGui::SliderFloat("Light Intensity", &intensity, 0.f, 1.f);

	ImGui::ColorEdit3("Light Color", &color.x);
}