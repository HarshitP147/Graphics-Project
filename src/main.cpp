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

#include "util/CheckError.h"
#include "util/LoadShaders.h"

#include <headers/camera.h>
#include <headers/house.h>
#include <headers/landscape.h>
#include <headers/skybox.h>
#include <headers/robot.h>

static GLFWwindow *window;

Camera *camera;

// Light settings
glm::vec3 lightPosition = glm::vec3(10.0f,100.0f, 100.0f);
glm::vec3 lightIntensity = glm::vec3(1e7);

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        camera->resetCamera();
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera->lookLeft();
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera->lookRight();
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        camera->moveForward();
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        camera->moveBackward();
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        camera->moveLeft();
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
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

    Robot rb(glm::vec3(0, 0, 0), 0, lightPosition, lightIntensity);

    Robot r1(glm::vec3(100, 0, 100), 20, lightPosition, lightIntensity);
    Robot r2(glm::vec3(-100, 0, -100), -35, lightPosition, lightIntensity);
    Robot r3(glm::vec3(100, 0, -100), 44, lightPosition, lightIntensity);
    Robot r4(glm::vec3(31,0, 89), 93, lightPosition, lightIntensity);
    Robot r5(glm::vec3(-44, 0, -74), -134, lightPosition, lightIntensity);

    Landscape ls1, ls2(glm::vec3(-100, 0, -100));
    Landscape ls3(glm::vec3(100, 0, 100));
    Landscape ls4(glm::vec3(-100, 0, 100));
    Landscape ls5(glm::vec3(100, 0, -100));

    House h1(glm::vec3(25, 0, 25));
    House h2(glm::vec3(-55, 0, 15));
    House h3(glm::vec3(-92, 0, -39));
    House h4(glm::vec3(-83, 0, 28));
    House h5(glm::vec3(56, 0, 03));
    House h6(glm::vec3(12,0, 45));

    static double lastTime = glfwGetTime();
    float time = 0.0f;
    float fTime = 0.0f;
    unsigned long frames = 0;

    do {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update animation states
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        time += deltaTime;
        rb.update(time);
        r1.update(time);
        r2.update(time);
        r3.update(time);
        r4.update(time);
        r5.update(time);

        glm::mat4 viewMatrix = camera->getViewMatrix();
        glm::mat4 projectionMatrix = camera->getProjectionMatrix();

        glm::mat4 vp = projectionMatrix * viewMatrix;

        glm::vec3 cameraPosition = camera->getCameraPosition();

        glm::mat4 skyBoxVP = projectionMatrix * glm::mat4(glm::mat3(viewMatrix));

        sb.render(skyBoxVP);

        // Rendering the landscape
        ls1.render(vp);
        ls2.render(vp);
        ls3.render(vp);
        ls4.render(vp);
        ls5.render(vp);

        rb.render(vp);
        r1.render(vp);
        r2.render(vp);
        r3.render(vp);
        r4.render(vp);
        r5.render(vp);

        h1.render(vp);
        h2.render(vp);
        h3.render(vp);
        h4.render(vp);
        h5.render(vp);
        h6.render(vp);

        // Frames tracking
        frames += 1;
        fTime += deltaTime;
        if (fTime >= 2.0f) {
            float fps = frames / fTime;
            fTime = 0.0f;
            frames = 0;

            std::stringstream sstream;
            sstream << std::fixed << std::setprecision(2) << "Graphics Project: " << fps << " FPS";
            glfwSetWindowTitle(window, sstream.str().c_str());
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (!glfwWindowShouldClose(window));

    // Clear all the buffers that we created
    delete camera;

    glfwTerminate();

    return 0;
}