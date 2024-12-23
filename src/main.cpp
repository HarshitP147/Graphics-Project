#include <headers/grid.h>
#include <headers/skybox.h>
#include <headers/camera.h>

#include <iostream>
#include <vector>
using std::vector;

#include <GLFW/glfw3.h>


static GLFWwindow *window;

Camera *camera;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode){
    if(key==GLFW_KEY_ESCAPE && action==GLFW_PRESS){
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
        camera->resetCamera();
    }

    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        camera->lookLeft();
    }

    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        camera->lookRight();
    }

    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        camera->moveForward();
    }

    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        camera->moveBackward();
    }

    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
        camera->moveLeft();
    }

    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
        camera->moveRight();
    }

}


int main() {

    // Initialize the window object
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a new window
    window = glfwCreateWindow(1280, 720, "Graphics Project", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to open a GLFW window." << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        std::cerr << "Failed to initialize OpenGL context." << std::endl;
        return -1;
    }

	glClearColor(0.2f, 0.2f, 0.25f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);

    // Handle input events
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(window, key_callback);

    // Now we initialize our objects
    camera = new Camera();

    Skybox sb("../src/assets/sky.png");

    Grid gd;

    do{
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 viewMatrix = camera->getViewMatrix();
        glm::mat4 projectionMatrix = camera->getProjectionMatrix();

        glm::mat4 vp = projectionMatrix * viewMatrix;

        glm::vec3 cameraPosition = camera->getCameraPosition();

        glm::mat4 skyBoxVP = projectionMatrix * glm::mat4(glm::mat3(viewMatrix));

        sb.render(vp, cameraPosition);

        gd.render(vp);

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while(!glfwWindowShouldClose(window));


    // Clear all the buffers that we created
    delete camera;


    glfwTerminate();

    return 0;
}