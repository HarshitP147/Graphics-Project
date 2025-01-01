

class Skybox{
    GLfloat vertex_buffer_data[72] = {
            // Back face
            1.0f, 1.0f, 1.0f,   // right top front
            -1.0f, 1.0f, 1.0f,  // left top front
            -1.0f, -1.0f, 1.0f, // left bottom front
            1.0f, -1.0f, 1.0f,  // right bottom front

            // Front face
            1.0f, -1.0f, -1.0f,  // right bottom back
            -1.0f, -1.0f, -1.0f, // left bottom back
            -1.0f, 1.0f, -1.0f,  // left top back
            1.0f, 1.0f, -1.0f,   // right top back

            // Left face
            -1.0f, -1.0f, -1.0f, // left bottom back
            -1.0f, -1.0f, 1.0f,  // right bottom front
            -1.0f, 1.0f, 1.0f,   // right top front
            -1.0f, 1.0f, -1.0f,  // left top back

            // Right face
            1.0f, 1.0f, 1.0f,   // right top front
            1.0f, -1.0f, 1.0f,  // right bottom front
            1.0f, -1.0f, -1.0f, // right bottom back
            1.0f, 1.0f, -1.0f,  // right top back

            // Top face
            1.0f, 1.0f, 1.0f,   // right top front
            1.0f, 1.0f, -1.0f,  // right top back
            -1.0f, 1.0f, -1.0f, // left top back
            -1.0f, 1.0f, 1.0f,  // left top front

            // Bottom face
            1.0f, -1.0f, -1.0f,  // right bottom back
            1.0f, -1.0f, 1.0f,   // right bottom front
            -1.0f, -1.0f, 1.0f,  // left bottom front
            -1.0f, -1.0f, -1.0f, // left bottom back
    };

    GLfloat color_buffer_data[72] = {
            // Front, red
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,

            // Back, yellow
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,

            // Left, green
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            0.0f,

            // Right, cyan
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,

            // Top, blue
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,

            // Bottom, magenta
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
        };

    GLuint index_buffer_data[36] = {
            // 12 triangle faces of a box
            0,
            1,
            2,
            0,
            2,
            3,

            4,
            5,
            6,
            4,
            6,
            7,

            8,
            9,
            10,
            8,
            10,
            11,

            12,
            13,
            14,
            12,
            14,
            15,

            16,
            17,
            18,
            16,
            18,
            19,

            20,
            21,
            22,
            20,
            22,
            23,
        };

    GLfloat uv_buffer_data[72] = {
        // -Z
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        // +Z
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,

        // +X
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,

        // -X
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,

        // +Y
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,

        // -Y
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,

    };

    // OpenGL buffers
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint colorBufferID;
    GLuint uvBufferID;
    std::vector<GLuint> textureIDs;

    // Shader variable IDs
    GLuint mvpMatrixID;
    GLuint textureSamplerID;
    GLuint programID;

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
        Skybox(){
            // Initialize the variables and the skybox

            // Create a vertex array object
            glGenVertexArrays(1, &vertexArrayID);
            glBindVertexArray(vertexArrayID);

            // Create a vertex buffer object to store the vertex data
            glGenBuffers(1, &vertexBufferID);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

            // Create a vertex buffer object to store the color data
            glGenBuffers(1, &colorBufferID);
            glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
            glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

            // create a vertex buffer ohject to store the UV data
            glGenBuffers(1, &uvBufferID);
            glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
            glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

            // Create an index buffer object to store the index data that defines triangle faces
            glGenBuffers(1, &indexBufferID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

            // Create and compile our GLSL program from the shaders
            programID = LoadShadersFromFile("../src/shaders/skybox.vert", "../src/shaders/skybox.frag");
            if (programID == 0) {
                std::cerr << "Failed to load shaders." << std::endl;
            }

            // Get a handle for our "MVP" uniform
            mvpMatrixID = glGetUniformLocation(programID, "MVP");

            // Load the texture
            std::vector<std::string> faces = {
                "../src/assets/skybox/pz.jpg",
                "../src/assets/skybox/nz.jpg",
                "../src/assets/skybox/px.jpg",
                "../src/assets/skybox/nx.jpg",
                "../src/assets/skybox/py.jpg",
                "../src/assets/skybox/ny.jpg",
            };

            // textureID = LoadSkyboxTexture(faces)
            for(int i=0; i<faces.size(); i++){
                textureIDs.push_back(LoadTextureTileBox(faces[i].c_str()));
            }

            // Get a handle to texture sampler
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

            // Scale up the skybox by 100 times
            modelMatrix = glm::scale(modelMatrix, glm::vec3(10000.0f));

            // Rotate the matrix by -45 degrees around the Y axis
            modelMatrix = glm::rotate(modelMatrix, glm::radians(180.0f), glm::vec3(0, 1, 0));

            // Complete the MVP transform
            glm::mat4 mvp = cameraMatrix * modelMatrix;

            // ------------------------------------
            glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

            // Enable the UV buffer and texture sampler
            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

            // Bind the texture
            for(int i=0; i<textureIDs.size(); i++){
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
                glUniform1i(textureSamplerID, i);

                // Draw the box
                glDrawElementsBaseVertex(
                    GL_TRIANGLES,
                    6,
                    GL_UNSIGNED_INT,
                    (void *)(i * 6 * sizeof(GLuint)),
                    0
                );

                // glDrawElements(
                //     GL_TRIANGLES,    // mode
                //     36,              // number of indices
                //     GL_UNSIGNED_INT, // type
                //     (void *)0        // element array buffer offset
                // );

            }

            glBindVertexArray(0);
            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(2);
        }

        ~Skybox(){
            glDeleteBuffers(1, &vertexBufferID);
            glDeleteBuffers(1, &colorBufferID);
            glDeleteBuffers(1, &indexBufferID);
            glDeleteVertexArrays(1, &vertexArrayID);
            glDeleteBuffers(1, &uvBufferID);
            // glDeleteBuffers(1, &textureID);
            for(int i=0; i<textureIDs.size(); i++){
                glDeleteTextures(1, &textureIDs[i]);
                // glDeleteBuffers(1, &textureIDs[i]);
            }
            glDeleteProgram(programID);
        }
};