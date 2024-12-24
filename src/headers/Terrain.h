class Terrain{

    GLuint programID;
    GLuint mvpMatrixID;


    public:
        Terrain(){


            programID = LoadShadersFromFile("../src/shaders/terrain.vert", "../src/shaders/terrain.frag");
            if(programID == 0){
                std::cout << "Error loading shaders" << std::endl;
                exit(1);
            }

            mvpMatrixID = glGetUniformLocation(programID, "MVP");
        }

        void render(glm::mat4 cameraMatrix){
            glUseProgram(programID);


            glm::mat4 modelMatrix = glm::mat4(1.0f);

            // Perform any transformations if required

            glm::mat4 mvp = cameraMatrix * modelMatrix;

            glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);


            glUseProgram(0);
        }

        ~Terrain(){
            glDeleteProgram(programID);
        }
};