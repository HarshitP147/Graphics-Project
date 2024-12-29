#version 330 core

// Input
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec4 a_joint;
layout(location = 4) in vec4 a_weight;

// Output data, to be interpolated for each fragment
out vec3 worldPosition;
out vec3 worldNormal;
out vec2 uv;

uniform mat4 MVP;
uniform mat4 u_jointMatrix[100];

void main() {

    // World-space geometry
    worldPosition = vertexPosition;
    worldNormal = vertexNormal;

    // UV coordinates
    uv = vertexUV;


    mat4 skinMatrix =   a_weight.x * u_jointMatrix[int(a_joint.x)] +
                        a_weight.y * u_jointMatrix[int(a_joint.y)] +
                        a_weight.z * u_jointMatrix[int(a_joint.z)] +
                        a_weight.w * u_jointMatrix[int(a_joint.w)] ;

    // Transform vertex
    gl_Position = MVP * skinMatrix * vec4(vertexPosition, 1.0);

}