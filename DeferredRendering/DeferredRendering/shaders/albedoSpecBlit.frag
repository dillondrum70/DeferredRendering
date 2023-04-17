#version 450                          
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D _ColorTex;

uniform bool _Albedo;

void main(){    
    if(_Albedo)
    {
        FragColor = vec4(texture(_ColorTex, TexCoord).rgb, 1);
    }
    else
    {
        FragColor = vec4(vec3(texture(_ColorTex, TexCoord).a), 1);
    }
}