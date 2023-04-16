#ifndef BLUR_EFFECT_H
#define BLUR_EFFECT_H

#include "PostprocessEffect.h"

#include "Texture.h"

#include "../EW/Mesh.h"

class FramebufferObject;

class BlurEffect : public PostprocessEffect
{
public:

	BlurEffect(Shader* shader, std::string name, ew::Mesh* quadMesh);
	~BlurEffect();

	void ExposeImGui() override;

	void SetupShader(const std::vector<unsigned int>& colorBuffers) override;

	void SetParent(FramebufferObject* parent) override;
	void SetExtraPass(bool extra) { extraPass = extra; }

	unsigned int Blur(unsigned int colorBuffer, bool& horizontal);

protected:

	std::vector<FramebufferObject*> blurFbos;
	Texture blurBuffers[2];

	ew::Mesh* _quadMesh;

	int _samples = 5;
	float _blurStrength = 1;

	//boolean value logging whether or not the effect will have a last blur pass outside of SetupShader
	//Normal guassian blur will have an extra pass in main after SetupShader is called, but bloom effect for example will not
	bool extraPass = true;	
};

#endif
