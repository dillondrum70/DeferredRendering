#version 450                          
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoord;

uniform vec3 _BrightColor;
uniform float _BrightnessThreshold;

//Uniforms from application

struct Attenuation
{
    float constant;
    float linear;
    float quadratic;
};

uniform Attenuation _Attenuation;

struct PointLight
{
    vec3 pos;
    vec3 color;
    float intensity;
};

uniform PointLight _PointLight;

struct DirectionalLight
{
    vec3 dir;
    vec3 color;
    float intensity;
};

const int MAX_DIRECTIONAL_LIGHTS = 8;
uniform DirectionalLight _DirectionalLight[MAX_DIRECTIONAL_LIGHTS];
uniform int _UsedDirectionalLights;

struct Spotlight
{
    vec3 pos;
    vec3 dir;
    vec3 color;
    float intensity;

    float range;
    float minAngle;
    float maxAngle;
    float falloff;
};

const int MAX_SPOTLIGHTS = 8;
uniform Spotlight _Spotlight[MAX_SPOTLIGHTS];
uniform int _UsedSpotlights;

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
uniform vec3 _CamPos;
uniform bool _Phong;

in vec4 lightSpacePos;
uniform sampler2D _ShadowMap;

uniform float _MinBias;
uniform float _MaxBias;

uniform bool _EnablePCF;
uniform int _PCFSamples;

struct GBuffer {
    sampler2D position;
    sampler2D normal;
    sampler2D albedoSpecular;
};

uniform GBuffer _GBuffer;

uniform bool _Deferred;

//Functions

vec3 calculateDiffuse(float coefficient, vec3 lightDir, vec3 worldNormal, vec3 intensity)
{
    return coefficient * clamp(dot(lightDir, worldNormal), 0, 1) * intensity;
}

float calculatePhong(vec3 eyeDir, vec3 lightDir, vec3 worldNormal)
{
    return dot(normalize(reflect(lightDir, worldNormal)), eyeDir);
}

float calculateBlinnPhong(vec3 eyeDir, vec3 lightDir, vec3 worldNormal)
{
    vec3 sum = eyeDir + lightDir;
    vec3 halfVec = sum / length(sum);
    return dot(worldNormal, halfVec);
}

vec3 calculateSpecular(float coefficient, float angle, float shininess, vec3 intensity)
{
    return coefficient * pow(clamp(angle, 0, 1), shininess) * intensity;
}

float calculateAttenuationFactor(float dist, float constant, float linear, float quadratic)
{
    return 1 / (constant + (linear * dist) + (quadratic * dist * dist));
}

void pointLight(inout vec3 diffuse, inout vec3 specular, vec3 position, vec3 normal)
{
    vec3 intensityRGB = _PointLight.intensity * _PointLight.color * _Mat.color;   //Material color and light intensity/color
    float dist = distance(position, _PointLight.pos);    //distance between candidate point and light
    vec3 lightDir = normalize(_PointLight.pos - position);  //Direction to light
    float attenuationFactor = calculateAttenuationFactor(dist, _Attenuation.constant, _Attenuation.linear, _Attenuation.quadratic);   //Factor of how much light makes it based on distance

    //Diffuse Light
    diffuse += calculateDiffuse(_Mat.diffuseCoefficient, lightDir, normal, intensityRGB)
    * attenuationFactor;
    
    //Specular Light
    float angle = 0;    //What dot product to put in for specular (depending on if phong or blinn-phong it changes)
    vec3 eyeDir = normalize(_CamPos - position);   //Direction to viewer

    if(_Phong)    //Phong
    {
        angle = calculatePhong(eyeDir, -lightDir, normal);
    }
    else    //Blinn-Phong
    {
         angle = calculateBlinnPhong(eyeDir, lightDir, normal);
    }
    
    specular += calculateSpecular(_Mat.specularCoefficient, angle, _Mat.shininess, intensityRGB)
    * attenuationFactor;
}

void directionalLight(inout vec3 diffuse, inout vec3 specular, vec3 position, vec3 normal)
{
    for(int i = 0; i < _UsedDirectionalLights; i++)
    {
        vec3 intensityRGB = _DirectionalLight[i].intensity * _DirectionalLight[i].color * _Mat.color;   //Material color and light intensity/color
        vec3 lightDir = normalize(_DirectionalLight[i].dir);
    
        //Diffuse Light
        diffuse += calculateDiffuse(_Mat.diffuseCoefficient, -lightDir, normal, intensityRGB);
    
        //Specular Light
        float angle = 0;    //What dot product to put in for specular (depending on if phong or blinn-phong it changes)
        vec3 eyeDir = normalize(_CamPos - position);   //Direction to viewer

        if(_Phong)    //Phong
        {
            angle = calculatePhong(eyeDir, lightDir, normal);
        }
        else    //Blinn-Phong
        {         
            angle = calculateBlinnPhong(eyeDir, -lightDir, normal);
        }
    
        specular += calculateSpecular(_Mat.specularCoefficient, angle, _Mat.shininess, intensityRGB);
    }
}

void calculateSpotlight(inout vec3 diffuse, inout vec3 specular, vec3 position, vec3 normal)
{
    for(int i = 0; i < _UsedSpotlights; i++)
    {
        vec3 intensityRGB = _Spotlight[i].intensity * _Spotlight[i].color * _Mat.color;   //Material color and light intensity/color
        float dist = distance(position, _Spotlight[i].pos);    //distance between candidate point and light
        vec3 lightDir = normalize(_Spotlight[i].pos - position);  //Direction to light
        float attenuationFactor = calculateAttenuationFactor(dist, _Attenuation.constant, _Attenuation.linear, _Attenuation.quadratic);   //Factor of how much light makes it based on distance

        vec3 fragDir = normalize(position - _Spotlight[i].pos);
        float fragAngle = dot(normalize(_Spotlight[i].dir), fragDir);
        float angularAttentuation = pow(max(min(((fragAngle - _Spotlight[i].maxAngle) / (_Spotlight[i].minAngle - _Spotlight[i].maxAngle)), 1), 0), _Spotlight[i].falloff) * _Spotlight[i].range;

        //Diffuse Light
        diffuse += calculateDiffuse(_Mat.diffuseCoefficient, lightDir, normal, intensityRGB)
        * attenuationFactor * angularAttentuation; 

        //Specular Light
        float angle = 0;    //What dot product to put in for specular (depending on if phong or blinn-phong it changes)
        vec3 eyeDir = normalize(_CamPos - position);   //Direction to viewer

        if(_Phong)    //Phong
        {
            angle = calculatePhong(eyeDir, -lightDir, normal);
        }
        else    //Blinn-Phong
        {
            angle = calculateBlinnPhong(eyeDir, lightDir, normal);
        }
    
        specular += calculateSpecular(_Mat.specularCoefficient, angle, _Mat.shininess, intensityRGB)
        * attenuationFactor * angularAttentuation;
    }
}

float calcShadow(sampler2D shadowMap, vec4 lightSpacePoint, float bias)
{
    vec3 sampleCoord = lightSpacePoint.xyz / lightSpacePoint.w;
    sampleCoord = sampleCoord * .5 + .5;

    if(sampleCoord.z > 1.0)
    {
        return 0.0f;
    }
    
    float mapDepth = texture(shadowMap, sampleCoord.xy).r;
    float depth = sampleCoord.z - bias;

    return step(mapDepth, depth);
}

float calcPCF(sampler2D shadowMap, vec4 lightSpacePoint, float bias, int samples)
{
    float totalShadow = 0;
    vec2 texOffset = 1.0 / textureSize(_ShadowMap, 0);

    vec3 sampleCoord = lightSpacePoint.xyz / lightSpacePoint.w;
    sampleCoord = sampleCoord * .5 + .5;

    if(sampleCoord.z > 1.0)
    {
        return 0.0f;
    }

    float depth = sampleCoord.z - bias;

    for(int y = -samples; y <= samples; y++)
    {
        for(int x = -samples; x <= samples; x++)
        {
            vec2 uv = sampleCoord.xy + vec2(x * texOffset.x, y * texOffset.y);
            totalShadow += step(texture(shadowMap, uv).r, depth - bias);
        }
    }

    int sampleSide = samples + samples + 1;

    return totalShadow / (sampleSide * sampleSide);
}

void main()
{   
    vec3 position = texture(_GBuffer.position, TexCoord).rgb;
    vec3 normal = texture(_GBuffer.normal, TexCoord).rgb;
    vec3 albedo = texture(_GBuffer.albedoSpecular, TexCoord).rgb;
    float specFloat = texture(_GBuffer.albedoSpecular, TexCoord).a;

    //Ambient Light
    vec3 ambient = _Mat.ambientCoefficient * _Mat.color * albedo;

    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);

    //Point Light diffuse and specular
    pointLight(diffuse, specular, position, normal);

    //Directional light diffuse and specular
    //directionalLight(diffuse, specular, position, normal);

    //Spotlight diffuse and specular
    //calculateSpotlight(diffuse, specular, position, normal);

    float bias = max(_MaxBias * (1.0 - dot(normal, -_DirectionalLight[0].dir)), _MinBias);
    float shadow = 0;
////////////////// Shadows disabled
//    if(_EnablePCF)
//    {
//        shadow = calcPCF(_ShadowMap, lightSpacePos, bias, _PCFSamples);
//    }
//    else
//    {
//        shadow = calcShadow(_ShadowMap, lightSpacePos, bias);
//    }

    diffuse *=  albedo;
    
    FragColor = vec4(ambient + ((diffuse + specular) * (1.0 - shadow)), 1.0f);
    

    //FragColor = vec4(vert_out.UV.x, vert_out.UV.y, 0, 1);
    //FragColor = vec4(normal, 1);
    float brightness = dot(FragColor.rgb, _BrightColor);
    if(brightness > _BrightnessThreshold)
    {
        BrightColor = vec4(FragColor.rgb, 1.0);
    }
    else
    {
        BrightColor = vec4(0, 0, 0, 1.0);
    }
}
