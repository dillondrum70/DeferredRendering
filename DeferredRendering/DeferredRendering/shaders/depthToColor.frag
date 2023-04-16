#version 450                          
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D _ColorTex;

//uniform float _Near;
//uniform float _Far;
//
//uniform float _Width;
//uniform float _Height;

void main(){         
    FragColor = texture(_ColorTex, TexCoord);
}