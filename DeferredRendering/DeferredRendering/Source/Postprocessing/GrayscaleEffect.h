#ifndef GRAYSCALE_EFFECT_H
#define GRAYSCALE_EFFECT_H

#include "PostprocessEffect.h"

class GrayscaleEffect : public PostprocessEffect
{
public:

	GrayscaleEffect(Shader* shader, std::string name) : PostprocessEffect(shader, name) {};

protected:
};

#endif
