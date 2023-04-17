#version 450                          
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in struct Vertex
{
    vec3 Normal;
    vec3 WorldPos;
    //vec3 WorldNormal;
    vec2 UV;
    mat3 TBN;   //Tangent to worldspace change of basis transform
}vert_out;


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

const int MAX_POINT_LIGHTS = 250;
uniform PointLight _PointLight[MAX_POINT_LIGHTS];
uniform int _UsedPointLights;

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

struct Texture
{
    vec2 scaleFactor;
    vec2 offset;
    bool hasNormal;
    sampler2D texSampler;
    sampler2D normSampler;
};

uniform Texture _CurrentTexture;

in vec4 lightSpacePos;
uniform sampler2D _ShadowMap;

uniform float _MinBias;
uniform float _MaxBias;

uniform bool _EnablePCF;
uniform int _PCFSamples;

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

void pointLights(inout vec3 diffuse, inout vec3 specular, vec3 normal)
{
    for(int i = 0; i < _UsedPointLights; i++)
    {
        vec3 intensityRGB = _PointLight[i].intensity * _PointLight[i].color * _Mat.color;   //Material color and light intensity/color
        float dist = distance(vert_out.WorldPos, _PointLight[i].pos);    //distance between candidate point and light
        vec3 lightDir = normalize(_PointLight[i].pos - vert_out.WorldPos);  //Direction to light
        float attenuationFactor = calculateAttenuationFactor(dist, _Attenuation.constant, _Attenuation.linear, _Attenuation.quadratic);   //Factor of how much light makes it based on distance

        //Diffuse Light
        diffuse += calculateDiffuse(_Mat.diffuseCoefficient, lightDir, normal, intensityRGB)
        * attenuationFactor;
    
        //Specular Light
        float angle = 0;    //What dot product to put in for specular (depending on if phong or blinn-phong it changes)
        vec3 eyeDir = normalize(_CamPos - vert_out.WorldPos);   //Direction to viewer

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
}

void directionalLight(inout vec3 diffuse, inout vec3 specular, vec3 normal)
{
    for(int i = 0; i < _UsedDirectionalLights; i++)
    {
        vec3 intensityRGB = _DirectionalLight[i].intensity * _DirectionalLight[i].color * _Mat.color;   //Material color and light intensity/color
        vec3 lightDir = normalize(_DirectionalLight[i].dir);
    
        //Diffuse Light
        diffuse += calculateDiffuse(_Mat.diffuseCoefficient, -lightDir, normal, intensityRGB);
    
        //Specular Light
        float angle = 0;    //What dot product to put in for specular (depending on if phong or blinn-phong it changes)
        vec3 eyeDir = normalize(_CamPos - vert_out.WorldPos);   //Direction to viewer

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

void calculateSpotlight(inout vec3 diffuse, inout vec3 specular, vec3 normal)
{
    for(int i = 0; i < _UsedSpotlights; i++)
    {
        vec3 intensityRGB = _Spotlight[i].intensity * _Spotlight[i].color * _Mat.color;   //Material color and light intensity/color
        float dist = distance(vert_out.WorldPos, _Spotlight[i].pos);    //distance between candidate point and light
        vec3 lightDir = normalize(_Spotlight[i].pos - vert_out.WorldPos);  //Direction to light
        float attenuationFactor = calculateAttenuationFactor(dist, _Attenuation.constant, _Attenuation.linear, _Attenuation.quadratic);   //Factor of how much light makes it based on distance

        vec3 fragDir = normalize(vert_out.WorldPos - _Spotlight[i].pos);
        float fragAngle = dot(normalize(_Spotlight[i].dir), fragDir);
        float angularAttentuation = pow(max(min(((fragAngle - _Spotlight[i].maxAngle) / (_Spotlight[i].minAngle - _Spotlight[i].maxAngle)), 1), 0), _Spotlight[i].falloff) * _Spotlight[i].range;

        //Diffuse Light
        diffuse += calculateDiffuse(_Mat.diffuseCoefficient, lightDir, normal, intensityRGB)
        * attenuationFactor * angularAttentuation; 

        //Specular Light
        float angle = 0;    //What dot product to put in for specular (depending on if phong or blinn-phong it changes)
        vec3 eyeDir = normalize(_CamPos - vert_out.WorldPos);   //Direction to viewer

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
    //TODO
    //Multiply by intensity (apply to X and Y value of normal so 0 will have normal pointing straight out in worldspace normal)
    vec2 uv = (vert_out.UV + _CurrentTexture.offset) * _CurrentTexture.scaleFactor;
    vec3 normal = vec3(0, 0, 1);

    if(_CurrentTexture.hasNormal)
    {
        normal =  (texture(_CurrentTexture.normSampler, uv).rgb * 2) - 1;
    }

    normal *= vec3(_Mat.normalIntensity, _Mat.normalIntensity, 1);
    normal = vert_out.TBN * normal;

    //Ambient Light
    vec3 ambient = _Mat.ambientCoefficient * _Mat.color;

    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);

    //Point Light diffuse and specular
    pointLights(diffuse, specular, normal);

    //Directional light diffuse and specular
    directionalLight(diffuse, specular, normal);

    //Spotlight diffuse and specular
    calculateSpotlight(diffuse, specular, normal);

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

    FragColor = texture(_CurrentTexture.texSampler, uv) * vec4(ambient + ((diffuse + specular) * (1.0 - shadow)), 1.0f);

    

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
