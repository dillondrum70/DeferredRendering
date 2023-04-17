#include "Texture.h"

#include "stb_image.h"

GLuint Texture::CreateTexture(const char* texFilePath)
{
	//Textures
	//////////////////////////
	glActiveTexture(texNumber);

	//Create texture name
	glGenTextures(1, &texture);

	//Make it a 2D texture
	glBindTexture(GL_TEXTURE_2D, texture);

	//Use if texture is vertically flipped
	//stbi_set_flip_vertically_on_load(true);

	

	//Load in our texture data from the file path
	textureData = stbi_load(texFilePath, &dimensions.x, &dimensions.y, &texFileChannels, desiredChannels);

	//Set texture data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dimensions.x, dimensions.y, 0, format, type, textureData);

	//Set wrapping behavior
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, verticalWrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, horizontalWrapMode);

	//Set filtering behavior
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);

	glGenerateMipmap(GL_TEXTURE_2D);

	return texture;
}

GLuint Texture::CreateTexture(GLenum internalFormat, unsigned int width, unsigned int height, GLenum format, GLenum type)
{
	Destroy();

	dimensions = glm::vec2(width, height);

	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return texture;
}

void Texture::Destroy()
{
	glDeleteTextures(1, &texture);
}

void Texture::ExposeImGui()
{
	glActiveTexture(texNumber);

	//Tiling
	ImGui::SliderFloat2("Texture Scale", &scaleFactor.x, .01, 10);

	//Scroll Speed
	ImGui::SliderFloat2("Scroll Speed", &scrollSpeed.x, 0, 10);

	//Vertical Wrapping
	int lastVertWrap = currentVertWrap;

	if (ImGui::BeginCombo("Vertical Wrap Mode", wrapModes[currentVertWrap], ImGuiComboFlags_None))
	{
		for (int i = 0; i < IM_ARRAYSIZE(wrapModes); i++)
		{
			bool selected = wrapModes[currentVertWrap] == wrapModes[i];

			if (ImGui::Selectable(wrapModes[i], selected))
			{
				currentVertWrap = i;
			}
				
			if (selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (currentVertWrap != lastVertWrap)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapEnumModes[currentVertWrap]);
	}

	//Horizontal Wrapping
	int lastHorizWrap = currentHorizWrap;

	if (ImGui::BeginCombo("Horizontal Wrap Mode", wrapModes[currentHorizWrap], ImGuiComboFlags_None))
	{
		for (int i = 0; i < IM_ARRAYSIZE(wrapModes); i++)
		{
			bool selected = wrapModes[currentHorizWrap] == wrapModes[i];

			if (ImGui::Selectable(wrapModes[i], selected))
			{
				currentHorizWrap = i;
			}

			if (selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (currentHorizWrap != lastHorizWrap)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapEnumModes[currentHorizWrap]);
	}

	//Mag Filter
	int lastMagFilter = currentMagFilter;

	if (ImGui::BeginCombo("Mag Filter", filterModes[currentMagFilter], ImGuiComboFlags_None))
	{
		for (int i = 0; i < IM_ARRAYSIZE(filterModes); i++)
		{
			bool selected = filterModes[currentMagFilter] == filterModes[i];

			if (ImGui::Selectable(filterModes[i], selected))
			{
				currentMagFilter = i;
			}

			if (selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (currentMagFilter != lastMagFilter)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterEnumModes[currentMagFilter]);
	}

	//MinFilter
	int lastMinFliter = currentMinFilter;

	if (ImGui::BeginCombo("Min Filter", filterModes[currentMinFilter], ImGuiComboFlags_None))
	{
		for (int i = 0; i < IM_ARRAYSIZE(filterModes); i++)
		{
			bool selected = filterModes[currentMinFilter] == filterModes[i];

			if (ImGui::Selectable(filterModes[i], selected))
			{
				currentMinFilter = i;
			}

			if (selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (currentMinFilter != lastMinFliter)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterEnumModes[currentMinFilter]);
	}
}