#version 330 core

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D textureSampler;

// Lighting parameters
uniform vec3 lightDirection = normalize(vec3(-0.5, -1.0, -0.3)); // Directional light
uniform vec3 lightIntensity = vec3(1.0, 1.0, 1.0); // Light intensity
uniform vec3 ambientColor = vec3(0.2, 0.2, 0.2);   // Ambient light

void main() {
    // Sample the texture color
    vec3 baseColor = texture(textureSampler, texCoord).rgb;

    // Calculate ambient lighting
    vec3 ambient = ambientColor * baseColor;

    // Calculate diffuse lighting
    vec3 normal = normalize(vec3(0.0, 0.0, 1.0)); // Use a default normal for now
    float diffuseFactor = max(dot(normal, -lightDirection), 0.0);
    vec3 diffuse = diffuseFactor * lightIntensity * baseColor;

    // Combine lighting components
    vec3 finalColor = ambient + diffuse;

    // Output final color
    FragColor = vec4(finalColor, 1.0);

    // FragColor = vec4(baseColor, 1.0);

}
