#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <util/LoadShaders.h>
#include <vector>

class Grid {
    private:
        GLuint gridVAO, gridVBO;
        GLuint shaderProgram;
        GLint mvpLocation;  // Uniform location for the MVP matrix

    public:
        Grid() {
            // Shader initialization
            shaderProgram = LoadShadersFromFile("../src/shaders/grid.vert", "../src/shaders/grid.frag");
            // shaderProgram = LoadShadersFromFile("../src/shaders/grid.vert", "../src/shaders/grid.frag");
            mvpLocation = glGetUniformLocation(shaderProgram, "MVP");

            // Grid data
            const int grid_size = 10;
            const float grid_spacing = 250.0f;
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

        void render(const glm::mat4& mvp) {
            glUseProgram(shaderProgram);
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