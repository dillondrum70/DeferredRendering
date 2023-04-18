#version 450   
layout (location = 0) in vec3 vPos;  
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) in vec3 vTangent;

out vec2 TexCoord;

uniform mat4 _Model;
uniform mat4 _View;
uniform mat4 _Projection;

void main()
{
	TexCoord = vTexCoord;

	//Just place the sphere, no other information needed
	gl_Position = _Projection * _View * _Model * vec4(vPos, 1);
}