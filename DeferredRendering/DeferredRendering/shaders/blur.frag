#version 450                          
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D _ColorTex;

uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

uniform float _BlurStrength;

void main()
{             

    //Source: https://learnopengl.com/Advanced-Lighting/Bloom
    vec2 tex_offset = 1.0 / textureSize(_ColorTex, 0); // gets size of single texel
    vec3 result = texture(_ColorTex, TexCoord).rgb * weight[0]; // current fragment's contribution

    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(_ColorTex, TexCoord + vec2(tex_offset.x * i * _BlurStrength, 0.0)).rgb * weight[i];
            result += texture(_ColorTex, TexCoord - vec2(tex_offset.x * i * _BlurStrength, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(_ColorTex, TexCoord + vec2(0.0, tex_offset.y * i * _BlurStrength)).rgb * weight[i];
            result += texture(_ColorTex, TexCoord - vec2(0.0, tex_offset.y * i * _BlurStrength)).rgb * weight[i];
        }
    }

    FragColor = vec4(result, 1.0);
}