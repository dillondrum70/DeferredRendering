#version 450                          
layout (location = 0) out vec4 gBufPosition;
layout (location = 1) out vec4 gBufNormal;
layout (location = 2) out vec4 gBufAlbedoSpecular;

in struct Vertex
{
    vec3 Normal;
    vec3 WorldPos;
    //vec3 WorldNormal;
    vec2 UV;
    mat3 TBN;   //Tangent to worldspace change of basis transform
}vert_out;

struct Material
{
    vec3 color;

    float ambientCoefficient;
    float diffuseCoefficient;
    float specularCoefficient;
    float shininess;
    float normalIntensity;
};

uniform Material _Mat;

struct Texture
{
    vec2 scaleFactor;
    vec2 offset;
    bool hasNormal;
    bool hasSpecular;
    sampler2D texSampler;
    sampler2D normSampler;
    sampler2D specSampler;
};

uniform Texture _CurrentTexture;

void main()
{   
    vec2 uv = (vert_out.UV + _CurrentTexture.offset) * _CurrentTexture.scaleFactor;
    vec3 normal = vec3(0, 0, 1);

    if(_CurrentTexture.hasNormal)
    {
        normal =  (texture(_CurrentTexture.normSampler, uv).rgb * 2) - 1;
    }

    normal *= vec3(_Mat.normalIntensity, _Mat.normalIntensity, 1);
    normal = vert_out.TBN * normal;

    ///// OpenGl doesn't spit anything out through these unless they are vec4s even though vec3s would be more space efficient
	gBufPosition = vec4(vert_out.WorldPos, 1);

    gBufNormal = vec4(normalize(normal), 1);
    /////

    gBufAlbedoSpecular.rgb = texture(_CurrentTexture.texSampler, uv).rgb;

    if(_CurrentTexture.hasSpecular)
    {
        gBufAlbedoSpecular.a = texture(_CurrentTexture.specSampler, uv).r;
    }
    else
    {
        gBufAlbedoSpecular.a = 0;
    }
}