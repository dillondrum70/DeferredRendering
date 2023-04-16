//Author: Eric Winebrenner

#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ewMath.h"

namespace ew {
	struct Transform {
		glm::vec3 position = glm::vec3(0);
		glm::quat rotation = glm::identity<glm::quat>();
		glm::vec3 scale = glm::vec3(1);

		glm::mat4 getModelMatrix() {
			return ew::translate(position) * glm::toMat4(rotation) * ew::scale(scale);
		}
		void reset() {
			position = glm::vec3(0);
			rotation = glm::vec3(0);
			scale = glm::vec3(1);
		}
	};
}
