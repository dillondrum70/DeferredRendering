#include "BlurEffect.h"

#include "GLFW/glfw3.h"

#include "FramebufferObject.h"



BlurEffect::BlurEffect(Shader * shader, std::string name, ew::Mesh* quadMesh) : PostprocessEffect(shader, name)
{
	_quadMesh = quadMesh;

	for (int i = 0; i < 2; i++)
	{
		FramebufferObject* fbo = new FramebufferObject();
		fbo->Create();
		blurFbos.push_back(fbo);
	}
}

BlurEffect::~BlurEffect()
{
	//Delete temp fbos
	for (int i = blurFbos.size() - 1; i >= 0; i--)
	{
		if (blurFbos[i] != nullptr)
		{
			delete blurFbos[i];
			blurFbos[i] = nullptr;
		}
	}

	blurFbos.clear();
}

void BlurEffect::ExposeImGui()
{
	PostprocessEffect::ExposeImGui();

	ImGui::SliderInt("Samples", &_samples, 1, 10);
	ImGui::SliderFloat("Blur Strength", &_blurStrength, 0, 10);
}


void BlurEffect::SetupShader(const std::vector<unsigned int>& colorBuffers)
{
	//PostprocessEffect::SetupShader(colorBuffers);

	/*_shader->use();
	_shader->setInt("_ColorTex", colorBuffers[0]);
	_shader->setFloat("_BlurStrength", _blurStrength);
	_shader->setInt("horizontal", _horizontal);*/

	bool horizontal;
	unsigned int tex = Blur(colorBuffers[0], horizontal);

	//Blurred map
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//Set sampler2Ds for bloom shader
	_shader->use();
	_shader->setInt("_ColorTex", 1);
	_shader->setFloat("_BlurStrength", _blurStrength);
	_shader->setInt("horizontal", horizontal);
	//Bind default buffer so the next draw goes to the screen
	_parent->Unbind(_parent->GetDimensions());
}


void BlurEffect::SetParent(FramebufferObject* parent)
{
	PostprocessEffect::SetParent(parent);

	//Create buffers based on parent dimensions
	for (int i = 0; i < 2; i++)
	{
		blurBuffers[i].CreateTexture(GL_RGBA, _parent->GetDimensions().x, _parent->GetDimensions().y, GL_RGBA, GL_FLOAT);
	}
}

unsigned int BlurEffect::Blur(unsigned int colorBuffer, bool& horizontal)
{
	for (unsigned int i = 0; i < 2; i++)
	{
		blurFbos[i]->Bind();
		blurFbos[i]->AddColorAttachment(blurBuffers[i], GL_COLOR_ATTACHMENT0);
	}

	//Use blur shader
	unsigned int tex = colorBuffer;
	horizontal = true;
	_shader->use();
	_shader->setFloat("_BlurStrength", _blurStrength);
	for (unsigned int i = 0; i < _samples - (extraPass ? 1 : 0); i++)
	{
		//Bind current fbo
		blurFbos[horizontal]->Bind();

		//Bind last texture drawn to
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex);

		//Shader variables and draw
		_shader->setInt("horizontal", horizontal);
		_shader->setInt("_ColorTex", 1);
		_quadMesh->draw();

		//Switch blur direction for next pass
		horizontal = !horizontal;

		//Set texture to sample from in next step (whether another blur or bloom)
		tex = blurBuffers[!horizontal].GetTexture();
	}

	return tex;
}