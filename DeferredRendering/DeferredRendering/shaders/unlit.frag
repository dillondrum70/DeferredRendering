#version 450                          
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

uniform vec3 _Color;

uniform vec3 _BrightColor;
uniform float _BrightnessThreshold;

void main(){         
    FragColor = vec4(_Color,1.0f);
    
    float brightness = dot(FragColor.rgb, _BrightColor);
    if(brightness >= _BrightnessThreshold)
    {
        BrightColor = vec4(FragColor.rgb, 1.0);
    }
    else
    {
        BrightColor = vec4(0, 0, 0, 1.0);
    }
}
