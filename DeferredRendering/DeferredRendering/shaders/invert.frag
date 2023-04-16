#version 450                          
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoord;

uniform sampler2D _ColorTex;

void main(){         
    vec4 col = texture(_ColorTex, TexCoord);

    FragColor = vec4(1 - col.xyz, 1);
}