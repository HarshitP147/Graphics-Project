#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <GLFW/glfw3.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <obj/obj_loader.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tinygltf/tiny_gltf.h>

#include <iostream>
#include <string>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

#include "util/LoadShaders.h"
#include "util/CheckError.h"

#include <headers/landscape.h>
#include <headers/grid.h>
#include <headers/skybox.h>
#include <headers/camera.h>
#include <headers/tree.h>
#include <headers/robot.h>


static GLFWwindow *window;

Camera *camera;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
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

       // Handle input events
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(window, key_callback);

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        std::cerr << "Failed to initialize OpenGL context." << std::endl;
        return -1;
    }

	glClearColor(0.2f, 0.2f, 0.25f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Now we initialize our objects
    camera = new Camera();

    Skybox sb;

    Robot rb;

    // Robot r1(glm::vec3(100, 0, 100), 20);

    // Robot r2(glm::vec3(-100, 0, -100), -35);

    // Tree tr;
    Landscape ls;


    static double lastTime = glfwGetTime();
    float time = 0.0f;
    float fTime = 0.0f;
    unsigned long frames = 0;

    do{
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update animation states
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        time += deltaTime;
        rb.update(time);
        // r1.update(time);
        // r2.update(time);

        glm::mat4 viewMatrix = camera->getViewMatrix();
        glm::mat4 projectionMatrix = camera->getProjectionMatrix();

        glm::mat4 vp = projectionMatrix * viewMatrix;

        glm::vec3 cameraPosition = camera->getCameraPosition();

        glm::mat4 skyBoxVP = projectionMatrix * glm::mat4(glm::mat3(viewMatrix));

        sb.render(skyBoxVP);

        rb.render(vp);

        // r1.render(vp);
        // r2.render(vp);


        // tr.render(vp);
        ls.render(vp);

        // Frames tracking
        frames += 1;
        fTime += deltaTime;
        if(fTime >= 2.0f){
            float fps = frames / fTime;
            fTime = 0.0f;
            frames = 0;

            std::stringstream sstream;
            sstream << std::fixed << std::setprecision(2) << "Graphics Project: " << fps << " FPS";
            glfwSetWindowTitle(window, sstream.str().c_str());
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while(!glfwWindowShouldClose(window));


    // Clear all the buffers that we created
    delete camera;


    glfwTerminate();

    return 0;
}