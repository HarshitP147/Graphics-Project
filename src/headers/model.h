
class Model{
    std::vector<float> vertices;
    std::vector<float> indices;
    std::vector<float> uvBuffer;
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint uvBufferID;
    std::vector<GLuint> textureIDs;

    GLuint programID;
    GLuint mvpMatrixID;
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
        Model(const char *model_path, std::vector<std::string> textures = {}){
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_path)) {
                std::cerr << warn << std::endl;
                std::cerr << err << std::endl;
            }

            for(const auto &shape: shapes){
                for(const auto &index: shape.mesh.indices){
                    vertices.push_back(attrib.vertices[3 * index.vertex_index + 0]);
                    vertices.push_back(attrib.vertices[3 * index.vertex_index + 1]);
                    vertices.push_back(attrib.vertices[3 * index.vertex_index + 2]);

                    if (index.texcoord_index >= 0) {
                        uvBuffer.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
                        uvBuffer.push_back(attrib.texcoords[2 * index.texcoord_index + 1]);
                    }

                    indices.push_back(indices.size());
                }
            }


            glGenVertexArrays(1, &vertexArrayID);
            glBindVertexArray(vertexArrayID);

            glGenBuffers(1, &vertexBufferID);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

            glGenBuffers(1, &uvBufferID);
            glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
            glBufferData(GL_ARRAY_BUFFER, uvBuffer.size() * sizeof(float), uvBuffer.data(), GL_STATIC_DRAW);

            glGenBuffers(1, &indexBufferID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(float), indices.data(), GL_STATIC_DRAW);

            programID = LoadShadersFromFile("../src/shaders/model.vert", "../src/shaders/model.frag");

            mvpMatrixID = glGetUniformLocation(programID, "MVP");
            textureSamplerID = glGetUniformLocation(programID, "textureSampler");

            for (size_t i = 0; i < textures.size(); i++) {
                textureIDs.push_back(LoadTextureTileBox(textures[i].c_str()));
            }
        }

        void render(glm::mat4 cameraMatrix){
            glUseProgram(programID);

            glBindVertexArray(vertexArrayID);

            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER,vertexBufferID);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

            for(size_t i=0; i<textureIDs.size(); i++){
                glActiveTexture(GL_TEXTURE1+i);
                glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
                glUniform1i(textureSamplerID, 1+i);
            }

            // Bind the index buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

            glm::mat4 modelMatrix = glm::mat4(1.0f);

            // translate the model
            modelMatrix = glm::translate(modelMatrix, glm::vec3(100.0f, 0.0f, 100.0f));

            // Scale down the model
            modelMatrix = glm::scale(modelMatrix, glm::vec3(0.03f));

            // Rotate the model throuhg X axis by 90 degrees
            modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));

            glm::mat4 mvp = cameraMatrix * modelMatrix;

            glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

            glBindVertexArray(vertexArrayID);

            glDrawArrays(GL_TRIANGLES, 0, vertices.size());

            glBindVertexArray(0);
            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(2);

        }

        ~Model(){
            glDeleteBuffers(1, &vertexBufferID);
            glDeleteBuffers(1, &indexBufferID);
            glDeleteBuffers(1, &uvBufferID);
            glDeleteVertexArrays(1, &vertexArrayID);
            glDeleteProgram(programID);
            for (size_t i = 0; i < textureIDs.size(); i++) {
                glDeleteTextures(1, &textureIDs[i]);
            }
        }
};