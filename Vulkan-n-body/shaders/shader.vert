#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_PointSize = 2.0;

    gl_Position = vec4(inPosition.xy, 0.0, 1.0);
    fragColor = inColor.rgb;
}

// REMEMBER TO MANUALLY COMPILE!!