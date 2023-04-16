#version 450                          
layout (location = 0) in vec3 vPos;  
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) in vec3 vTangent;

out struct Vertex
{
    vec3 Normal;
    vec3 WorldPos;
    //vec3 WorldNormal;
    vec2 UV;
    mat3 TBN;   //Tangent to worldspace change of basis transform
}vert_out;

uniform mat4 _Model;
uniform mat4 _View;
uniform mat4 _Projection;

uniform mat4 _NormalMatrix;

uniform mat4 _LightViewProj;

out vec4 lightSpacePos;

void main(){    
    lightSpacePos = _LightViewProj * _Model * vec4(vPos, 1);

    vert_out.Normal = vNormal;
    vert_out.WorldPos = vec3(_Model * vec4(vPos, 1));

    //vert_out.WorldNormal = normalize(mat3(_NormalMatrix) * vert_out.Normal);
    vert_out.UV = vTexCoord;

    vec3 norm = normalize(mat3(_NormalMatrix) * vert_out.Normal);
    vert_out.TBN = mat3(
        vTangent,
        cross(norm, vTangent),
        norm
    );
    gl_Position = _Projection * _View * _Model * vec4(vPos,1);
}
