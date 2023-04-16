#version 450                          
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D _ColorTex;

uniform float _AberrationAmount;
uniform float _Time;
uniform float _FlickerSpeed;
uniform float _FlickerAmount;

void main(){        
    float flicker = (sin(_Time * _FlickerSpeed) + 1) / 2;
    flicker = 1 - (flicker * _FlickerAmount);

    float red = texture(_ColorTex, TexCoord + vec2(_AberrationAmount * flicker, 0)).r;
    float green = texture(_ColorTex, TexCoord).g;
    float blue = texture(_ColorTex, TexCoord + vec2(_AberrationAmount * flicker, 0)).b;

    FragColor = vec4(red, green, blue, 1);
}