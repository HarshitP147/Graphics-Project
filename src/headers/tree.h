class Tree {
    std::vector<float> vertices;
    std::vector<int> indices;
    std::vector<float> colorBuffer;

    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint colorBufferID;

    GLuint programID;
    GLuint mvpMatrixID;

public:
    Tree() {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        // Load OBJ file
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "../src/assets/models/tree/Tree.obj")) {
            std::cerr << "Error loading OBJ file: " << warn << err << std::endl;
            return;
        }

        for (const auto &shape : shapes) {
            for (const auto &index : shape.mesh.indices) {
                vertices.push_back(attrib.vertices[3 * index.vertex_index + 0]);
                vertices.push_back(attrib.vertices[3 * index.vertex_index + 1]);
                vertices.push_back(attrib.vertices[3 * index.vertex_index + 2]);

                indices.push_back(indices.size());

            }
        }

        CheckOpenGLErrors("Tree::Tree - loading obj");

        glGenVertexArrays(1, &vertexArrayID);
        glBindVertexArray(vertexArrayID);

        // Vertex buffer
        glGenBuffers(1, &vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        // Index buffer
        glGenBuffers(1, &indexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(float), indices.data(), GL_STATIC_DRAW);

        CheckOpenGLErrors("Tree::Tree - buffers binding");

        programID = LoadShadersFromFile("../src/shaders/tree.vert", "../src/shaders/tree.frag");
        if (programID == 0) {
            std::cerr << "Error loading shaders." << std::endl;
            return;
        }

        mvpMatrixID = glGetUniformLocation(programID, "MVP");

        CheckOpenGLErrors("Tree::Tree");
    }

    void render(glm::mat4 cameraMatrix) {
        glUseProgram(programID);

        glBindVertexArray(vertexArrayID);

        // Bind vertex buffer
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        // Bind index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

        CheckOpenGLErrors("Tree::render - before uniform");

        glm::mat4 modelMatrix = glm::mat4(1.0f);

        glm::mat4 mvp = cameraMatrix * modelMatrix;

        glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

        CheckOpenGLErrors("Tree::render - before draw");

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glDisableVertexAttribArray(0);

        CheckOpenGLErrors("Tree::render");
    }

    ~Tree() {
        glDeleteBuffers(1, &vertexBufferID);
        glDeleteBuffers(1, &colorBufferID);
        glDeleteBuffers(1, &indexBufferID);
        glDeleteVertexArrays(1, &vertexArrayID);
    }
};
