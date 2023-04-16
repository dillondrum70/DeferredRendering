#version 450                          
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoord;

uniform sampler2D _ColorTex;

uniform float _SampleDist;
uniform float _Threshold;

void main(){         
     vec4 col = texture(_ColorTex, TexCoord);
     vec4 sampleCol = texture(_ColorTex, TexCoord + vec2(_SampleDist, 0));

     if(distance(col.xyz, sampleCol.xyz) > _Threshold)
     {
        FragColor = vec4(vec3(0), 1);
     }
     else
     {
        FragColor = col;
     }
}