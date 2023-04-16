#version 450                          
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D _ColorTex;

uniform sampler2D _BlurredBrightTex;

void main()
{             
    vec3 col = texture(_ColorTex, TexCoord).rgb;
    vec3 brightCol = texture(_BlurredBrightTex, TexCoord).rgb;

    FragColor = vec4(col + brightCol, 1);
    //FragColor = vec4(brightCol, 1);
    //FragColor = vec4(col, 1);
}