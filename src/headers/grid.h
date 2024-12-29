const char* grid_vert_shader = R"(
#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(position, 1.0);
}
)";

const char* grid_frag_shader = R"(
#version 330 core

out vec4 color;

void main() {
    color = vec4(0.5, 0.5, 0.5, 1.0);
}
)";


class Grid {
    private:
        GLuint gridVAO, gridVBO;
        GLuint shaderProgram;
        GLint mvpLocation;  // Uniform location for the MVP matrix

    public:
        Grid() {
            // Shader initialization
            shaderProgram = LoadShadersFromString(grid_vert_shader, grid_frag_shader);
            mvpLocation = glGetUniformLocation(shaderProgram, "MVP");

            // Grid data
            const int grid_size = 10;
            const float grid_spacing = 10.0f;
            std::vector<float> vertices;
            for (int i = -grid_size; i <= grid_size; ++i) {
                // Lines parallel to X-axis
                vertices.push_back(-grid_size * grid_spacing);
                vertices.push_back(0);
                vertices.push_back(i * grid_spacing);
                vertices.push_back(grid_size * grid_spacing);
                vertices.push_back(0);
                vertices.push_back(i * grid_spacing);
                // Lines parallel to Z-axis
                vertices.push_back(i * grid_spacing);
                vertices.push_back(0);
                vertices.push_back(-grid_size * grid_spacing);
                vertices.push_back(i * grid_spacing);
                vertices.push_back(0);
                vertices.push_back(grid_size * grid_spacing);
            }

            // Setup grid VAO and VBO
            glGenVertexArrays(1, &gridVAO);
            glBindVertexArray(gridVAO);

            glGenBuffers(1, &gridVBO);
            glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            glBindVertexArray(0);
        }

        void render(const glm::mat4& cameraMatrix) {
            glUseProgram(shaderProgram);

            glm::mat4 model = glm::mat4(1.0f);

            glm::mat4 mvp = cameraMatrix * model;


            glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp[0][0]);

            glBindVertexArray(gridVAO);
            glDrawArrays(GL_LINES, 0, (10 * 2 + 1) * 4);  // Each grid line is rendered
            glBindVertexArray(0);
        }

        ~Grid() {
            glDeleteBuffers(1, &gridVBO);
            glDeleteVertexArrays(1, &gridVAO);
            glDeleteProgram(shaderProgram);
        }
};