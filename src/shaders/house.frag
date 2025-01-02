#version 330 core

in vec2 fragUV;
in vec3 fragNormal;
in vec3 fragPosition;

out vec4 FragColor;

uniform sampler2D textureSampler;

// lighting uniforms:
uniform vec3 lightPosition;
uniform vec3 lightIntensity;

void main() {
    // Basic texture color
    vec3 baseColor = texture(textureSampler, fragUV).rgb;

    // Ambient
    vec3 ambient = 0.7 * baseColor;

    // Diffuse
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightPosition - fragPosition);
    float diffFactor = max(dot(N, L), 0.0);
    vec3 diffuse = diffFactor * lightIntensity * baseColor;

    vec3 finalColor = ambient + diffuse;

    // Output
    FragColor = vec4(finalColor, 1.0);
}
