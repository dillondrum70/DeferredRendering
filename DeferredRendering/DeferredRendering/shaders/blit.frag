#version 450                          
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D _ColorTex;

void main(){         
    FragColor = texture(_ColorTex, TexCoord);
    //FragColor = vec4(TexCoord, 0, 1);
}