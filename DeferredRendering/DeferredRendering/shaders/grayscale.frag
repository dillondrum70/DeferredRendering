#version 450                          
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoord;

uniform sampler2D _ColorTex;

void main(){         
     vec4 col = texture(_ColorTex, TexCoord);

     float avg = (col.r + col.g + col.b) / 3;

     FragColor = vec4(vec3(avg), 1);
}