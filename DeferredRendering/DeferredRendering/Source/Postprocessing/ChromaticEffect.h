#ifndef CHROMATIC_EFFECT_H
#define CHROMATIC_EFFECT_H

#include "PostprocessEffect.h"

class ChromaticEffect : public PostprocessEffect
{
public:

	ChromaticEffect(Shader* shader, std::string name) : PostprocessEffect(shader, name) {};

	void ExposeImGui() override;

	void SetupShader(const std::vector<unsigned int>& colorBuffers) override;

protected:

	float _aberrationAmount = 0.01f;
	float _flickerSpeed = 10.f;
	float _flickerAmount = .1f;
};

#endif
