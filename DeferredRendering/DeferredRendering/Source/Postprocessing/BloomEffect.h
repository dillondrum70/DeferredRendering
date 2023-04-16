#ifndef BLOOM_EFFECT_H
#define BLOOM_EFFECT_H

#include "PostprocessEffect.h"
#include "BlurEffect.h"

#include "../EW/Mesh.h"

class FramebufferObject;

class BloomEffect : public PostprocessEffect
{
public:

	BloomEffect(Shader* shader, std::string name, BlurEffect* blurEffect);

	void ExposeImGui() override;

	void SetupShader(const std::vector<unsigned int>& colorBuffers) override;

	void SetParent(FramebufferObject* parent) override;

protected:

	//Must be a dynamic pointer object since undefined with foward declaration, otherwise we need to create a new one every frame
	//std::vector<FramebufferObject*> blurFbos;
	//ColorBuffer blurBuffers[2];

	//int _samples = 5;
	//float _blurStrength = 1;

	//ew::Mesh* _quadMesh;

	BlurEffect* _blurEffect;

};

#endif
