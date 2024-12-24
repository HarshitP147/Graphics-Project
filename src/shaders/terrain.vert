#version 330 core

// Input attributes from the vertex buffer
layout(location = 0) in vec3 vertexPosition; // Vertex position

// Uniforms
uniform mat4 MVP;    // Model-View-Projection matrix

void main() {
    // Transform the vertex position to clip space
    gl_Position = MVP * vec4(vertexPosition, 1.0);

    // Pass UV coordinates to the fragment shader
    uv = vertexUV;

}
