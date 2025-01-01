#version 330 core

in vec3 fragmentColor;
out vec4 FragColor;

void main() {
    vec3 color = fragmentColor;
    if (length(color) == 0.0) { // No color provided
        color = vec3(0.3, 0.7, 0.2); // Default green for the tree
    }
    FragColor = vec4(color, 1.0);
}
