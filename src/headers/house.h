class House{
    std::vector<float> vertices;
    std::vector<int> indices;
    std::vector<float> normals;
    std::vector<float> uvs;

    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint normalBufferID;
    GLuint uvBufferID;

    GLuint programID;
    GLuint mvpMatrixID;
    GLuint diffuseTextureObject = 0;
    GLuint diffuseTextureID;

    GLuint lightPositionID;
    GLuint lightIntensityID;

    glm::vec3 position;
    glm::vec3 lightPosition;
    glm::vec3 lightIntensity;
    GLfloat angle;

    private:
        GLuint LoadTextureTileBox(const char *texture_path){
            int width, height, channels;
            unsigned char *image = stbi_load(texture_path, &width, &height, &channels, 0);

            GLuint textureID;
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);

            if(image){
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            } else{
                std::cerr << "Error loading texture: " << texture_path << std::endl;
                return 0;
            }
            stbi_image_free(image);
            return textureID;
        }

    public:
        // House(glm::vec3 position=glm::vec3(0.0f), GLfloat angle=0.0f): position(position), angle(angle){
        House(glm::vec3 position=glm::vec3(0.0f), GLfloat angle=0.0f, glm::vec3 lightPosition=glm::vec3(10.0f,100.0f, 100.0f), glm::vec3 lightIntensity=glm::vec3(1e7)){
            this->position = position;
            this->angle = angle;
            this->lightPosition = lightPosition;
            this->lightIntensity = lightIntensity;

            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

            // Load OBJ file
            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "../src/assets/models/house/model.obj")) {
                std::cerr << "Error loading OBJ file: " << warn << err << std::endl;
                return;
            }

            for (const auto &shape : shapes) {
                for (const auto &index : shape.mesh.indices) {
                    vertices.push_back(attrib.vertices[3 * index.vertex_index + 0]);
                    vertices.push_back(attrib.vertices[3 * index.vertex_index + 1]);
                    vertices.push_back(attrib.vertices[3 * index.vertex_index + 2]);

                    if(index.texcoord_index >= 0){
                        uvs.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
                        uvs.push_back(attrib.texcoords[2 * index.texcoord_index + 1]);
                    } else{
                        uvs.push_back(0.0f);
                        uvs.push_back(0.0f);
                    }

                    if(index.normal_index >= 0){
                        normals.push_back(attrib.normals[3 * index.normal_index + 0]);
                        normals.push_back(attrib.normals[3 * index.normal_index + 1]);
                        normals.push_back(attrib.normals[3 * index.normal_index + 2]);
                    } else{
                        normals.push_back(0.0f);
                        normals.push_back(0.0f);
                        normals.push_back(0.0f);
                    }

                    indices.push_back(indices.size());

                }
            }

            glGenVertexArrays(1, &vertexArrayID);
            glBindVertexArray(vertexArrayID);

            // Vertex buffer
            glGenBuffers(1, &vertexBufferID);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

            // UV buffer data
            glGenBuffers(1, &uvBufferID);
            glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
            glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(float), uvs.data(), GL_STATIC_DRAW);

            // Normal buffer data
            glGenBuffers(1, &normalBufferID);
            glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
            glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);

            // Index buffer
            glGenBuffers(1, &indexBufferID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(float), indices.data(), GL_STATIC_DRAW);

            programID = LoadShadersFromFile("../src/shaders/house.vert", "../src/shaders/house.frag");
            if (programID == 0) {
                std::cerr << "Error loading shaders." << std::endl;
                return;
            }

            mvpMatrixID = glGetUniformLocation(programID, "MVP");

            // Load texture
            diffuseTextureID = glGetUniformLocation(programID, "diffuseTexture");

            lightPositionID = glGetUniformLocation(programID, "lightPosition");
            lightIntensityID = glGetUniformLocation(programID, "lightIntensity");

            if(!materials.empty()){
                std::string diffuseTexPath = materials[0].diffuse_texname;
                if(!diffuseTexPath.empty()){
                    diffuseTextureID = LoadTextureTileBox(diffuseTexPath.c_str());
                }
            } else{
                diffuseTextureObject = LoadTextureTileBox("../src/assets/models/house/Sci-Fi_Building_01_baseColor.png");
            }

            glBindVertexArray(0);
            CheckOpenGLErrors("House::House - loading obj");
        }

        void render(glm::mat4 cameraMatrix){
            glUseProgram(programID);

            glBindVertexArray(vertexArrayID);

            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);

            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);

            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);

            // Bind index buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

            glm::mat4 modelMatrix = glm::mat4(1.0f);

            // Translate the matrix
            modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, 5.0f, position.z));

            // Scale down the matrix
            modelMatrix = glm::scale(modelMatrix, glm::vec3(0.01f));

            // Rotate the matrix
            modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

            glm::mat4 mvp = cameraMatrix * modelMatrix;

            glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

            // Bind texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuseTextureObject);
            glUniform1i(diffuseTextureID, 0);

            glUniform3fv(lightPositionID, 1, &lightPosition[0]);
            glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);

            // Draw the object
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void *) 0);

            CheckOpenGLErrors("House::render - before uniform");

            glBindVertexArray(0);
            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(2);
        }

        ~House(){
            glDeleteBuffers(1, &vertexBufferID);
            glDeleteBuffers(1, &uvBufferID);
            glDeleteBuffers(1, &indexBufferID);
            glDeleteBuffers(1, &normalBufferID);
            glDeleteVertexArrays(1, &vertexArrayID);
            glDeleteProgram(programID);
        }
};