#version 330 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

out vec2 fragUV;
out vec3 fragNormal;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(inPosition, 1.0);

    fragUV = inUV;

    fragNormal = inNormal;
}
