class Hut {
    GLfloat vertex_buffer_data[72] = {
        // Front face
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,

        // Back face
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        // Top face
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,

        // Bottom face
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        // Right face
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,

        // Left face
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f
    };

    GLfloat color_buffer_data[72] = {
        // Front face
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,

        // Back face
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,

        // Top face
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,

        // Bottom face
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,

        // Right face
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,

        // Left face
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f
    };

    GLuint index_buffer_data[72] = {
        // Front face
        0, 1, 2,
        2, 3, 0,

        // Back face
        4, 5, 6,
        6, 7, 4,

        // Top face
        8, 9, 10,
        10, 11, 8,

        // Bottom face
        12, 13, 14,
        14, 15, 12,

        // Right face
        16, 17, 18,
        18, 19, 16,

        // Left face
        20, 21, 22,
        22, 23, 20
    };

     GLfloat uv_buffer_data[72] = {

            // Back (-Z) - Third square from the left on the bottom row
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,

            // Front (+Z)
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.0f,


            // Left (+X)
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,

            // Right (-X) - Fourth square from the left on the bottom row
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,


            // Right (+Y)
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.0f,


            // Bottom (-Y)
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,

        };


    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint colorBufferID;
    GLuint uvBufferID;
    GLuint textureID;

    // Shader buffers
    GLuint mvpMatrixID;
    GLuint programID;
    GLuint textureSamplerID;

    private:
         GLuint LoadTextureTileBox(const char *texture_path) {
            int w, h, channels;
            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            uint8_t *img = stbi_load(texture_path, &w, &h, &channels, 3);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // To tile textures on a box, we set wrapping to repeat
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);

            if (img) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
                glGenerateMipmap(GL_TEXTURE_2D);
            } else {
                std::cout << "Failed to load texture " << texture_path << std::endl;
            }
            stbi_image_free(img);

            return texture;
        }

    public:
        Hut(){
            glGenVertexArrays(1, &vertexArrayID);
            glBindVertexArray(vertexArrayID);

            glGenBuffers(1, &vertexBufferID);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

            glGenBuffers(1, &colorBufferID);
            glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
            glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

            glGenBuffers(1, &uvBufferID);
            glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
            glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

            glGenBuffers(1, &indexBufferID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

            programID = LoadShadersFromFile("../src/shaders/box.vert", "../src/shaders/box.frag");

            if (programID == 0) {
                std::cerr << "Failed to load shaders." << std::endl;
            }

            mvpMatrixID = glGetUniformLocation(programID, "MVP");

            textureID = LoadTextureTileBox("../src/assets/window.jpg");

            textureSamplerID = glGetUniformLocation(programID, "textureSampler");

        }

        void render(glm::mat4 cameraMatrix) {
            glUseProgram(programID);
            glBindVertexArray(vertexArrayID);

            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

            glm::mat4 modelMatrix = glm::mat4(1.0f);

            modelMatrix = glm::scale(modelMatrix, glm::vec3(100.0f));

            // Rotate the box
            // modelMatrix = glm::rotate(modelMatrix, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            // Rotate the box around X axis by 180 degrees
            modelMatrix = glm::rotate(modelMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

            glm::mat4 mvp = cameraMatrix * modelMatrix;

            glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

            // Enable the texture
            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);


            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glUniform1i(textureSamplerID, 0);

            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(2);
        }

        ~Hut(){
            glDeleteBuffers(1, &vertexBufferID);
            glDeleteBuffers(1, &indexBufferID);
            glDeleteBuffers(1, &colorBufferID);
            glDeleteBuffers(1, &uvBufferID);
            glDeleteTextures(1, &textureID);
            glDeleteVertexArrays(1, &vertexArrayID);
        }


};
