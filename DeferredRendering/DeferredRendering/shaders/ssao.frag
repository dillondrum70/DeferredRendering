#version 450                          
layout (location = 0) out float SSAO;

in vec2 TexCoord;

struct GBuffer {
    sampler2D position;
    sampler2D normal;
};

uniform GBuffer _GBuffer;

uniform sampler2D _TexNoise;

const int KERNEL_SIZE = 64;

uniform vec3 _Samples[KERNEL_SIZE];
uniform mat4 _Projection;

uniform vec2 _ScreenDimensions;

const vec2 noiseDimensions = vec2(4, 4);

void main()
{
    vec2 noiseScale = _ScreenDimensions / noiseDimensions;

    vec3 position = texture(_GBuffer.position, TexCoord).xyz;
    vec3 normal = texture(_GBuffer.normal, TexCoord).xyz;
    vec3 random = texture(_TexNoise, TexCoord).xyz;

    vec3 tangent = normalize(random - normal * dot(random, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for(int i = 0; i < KERNEL_SIZE; i++)
    {
        float radius = 1;
        float bias = .05;
        vec3 samplePos = position + (TBN * _Samples[i] * radius);

        vec4 offset = _Projection * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * .5 + .5;

        float sampleDepth = texture(_GBuffer.position, offset.xy).z;

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(position.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0);
    }

    occlusion = 1.0 - (occlusion / KERNEL_SIZE);
    SSAO = occlusion;
}