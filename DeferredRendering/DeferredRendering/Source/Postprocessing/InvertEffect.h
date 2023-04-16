#ifndef INVERT_EFFECT_H
#define INVERT_EFFECT_H

#include "PostprocessEffect.h"

class InvertEffect : public PostprocessEffect
{
public:

	InvertEffect(Shader* shader, std::string name) : PostprocessEffect(shader, name) {};

protected:
};

#endif
