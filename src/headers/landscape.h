class Landscape{
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> uvs;
    std::vector<GLuint> indices;

    glm::vec3 position;
    glm::vec3 lightPosition;
    glm::vec3 lightIntensity;

    // Vertex buffer object IDs
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint uvBufferID;

    GLuint textureID;

    // Shader variable IDs
    GLuint programID;
    GLuint mvpMatrixID;
    GLuint textureSamplerID;

    private:
        GLuint LoadTextureTileBox(const char *texture_path){
            int w, h, channels;
            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            unsigned char *img = stbi_load(texture_path, &w, &h, &channels, 3);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            if(img){
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
                glGenerateMipmap(GL_TEXTURE_2D);
            } else{
                std::cout << "Failed to load texture " << texture_path << std::endl;
            }
            stbi_image_free(img);

            return texture;
        }

    public:
        Landscape(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 lightPosition = glm::vec3(10.0f, 100.0f, 100.0f), glm::vec3 lightIntensity = glm::vec3(1e7)){
            this->position = position;
            this->lightPosition = lightPosition;
            this->lightIntensity = lightIntensity;

            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

            // Load OBJ file
            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "../src/assets/models/landscape/20241010_RC_002_LOD1.obj")) {
                std::cerr << "Error loading OBJ file: " << warn << err << std::endl;
                return;
            }

            for (const auto &shape : shapes) {
                for (const auto &index : shape.mesh.indices) {
                    vertices.push_back(attrib.vertices[3 * index.vertex_index + 0]);
                    vertices.push_back(attrib.vertices[3 * index.vertex_index + 1]);
                    vertices.push_back(attrib.vertices[3 * index.vertex_index + 2]);

                    uvs.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
                    uvs.push_back(1.0f-attrib.texcoords[2 * index.texcoord_index + 1]);

                    indices.push_back(indices.size());
                }
            }

            CheckOpenGLErrors("Landscape::Landscape - loading obj");

            glGenVertexArrays(1, &vertexArrayID);
            glBindVertexArray(vertexArrayID);

            // Vertex buffer
            glGenBuffers(1, &vertexBufferID);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

            // UV buffer
            glGenBuffers(1, &uvBufferID);
            glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
            glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(GLfloat), uvs.data(), GL_STATIC_DRAW);

            // Index buffer
            glGenBuffers(1, &indexBufferID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

            CheckOpenGLErrors("Landscape::Landscape - buffers binding");

            programID = LoadShadersFromFile("../src/shaders/landscape.vert", "../src/shaders/landscape.frag");
            if (programID == 0) {
                std::cerr << "Error loading shaders." << std::endl;
                return;
            }

            textureID = LoadTextureTileBox("../src/assets/models/landscape/20241010_RC_002_LOD1_u0_v0_diffuse.png");

            mvpMatrixID = glGetUniformLocation(programID, "MVP");
            textureSamplerID = glGetUniformLocation(programID, "textureSampler");

            CheckOpenGLErrors("Landscape::Landscape");
        }

        void render(glm::mat4 cameraMatrix){
            glUseProgram(programID);

            glBindVertexArray(vertexArrayID);

            // Bind vertex buffer
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

            // Bind UV buffer
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

            // Bind index buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

            CheckOpenGLErrors("Landscape::render - before mvp");

            glm::mat4 modelMatrix = glm::mat4(1.0f);

            modelMatrix = glm::translate(modelMatrix, position);

            glm::mat4 mvp = cameraMatrix * modelMatrix;

            glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);


            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glUniform1i(textureSamplerID, 0);

            // Set light data
            glUniform3fv(glGetUniformLocation(programID, "lightPosition"), 1, &lightPosition[0]);
            glUniform3fv(glGetUniformLocation(programID, "lightIntensity"), 1, &lightIntensity[0]);

            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void *)0);

            glBindVertexArray(0);
            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
        }

        ~Landscape(){
            glDeleteBuffers(1, &vertexBufferID);
            glDeleteBuffers(1, &indexBufferID);
            glDeleteBuffers(1, &uvBufferID);
            glDeleteVertexArrays(1, &vertexArrayID);
            glDeleteProgram(programID);
        }
};