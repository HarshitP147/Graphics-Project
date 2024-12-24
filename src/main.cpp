#include <headers/model.h>
#include <headers/grid.h>
#include <headers/skybox.h>
#include <headers/Hut.h>
#include <headers/camera.h>
#include <headers/Terrain.h>

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

    std::vector<std::string> texture_images = {
        "../src/assets/models/train/03.jpg",
        "../src/assets/models/train/AVE.JPG",
        "../src/assets/models/train/AVEENG.jpg",
        // "../src/assets/train/AVELEFT.jpg"
        // "../src/assets/train/AVERIGHT.jpg"
        // "../src/assets/train/BLANCO.jpg",
        "../src/assets/models/train/GRIS.jpg",
        "../src/assets/models/train/LATERAL.jpg",
        "../src/assets/models/train/RENFE.jpg",
        "../src/assets/models/train/RENFECIR.jpg",
        "../src/assets/models/train/V.jpg"
    };

    // Model maglev("../src/assets/train/AnyConv.com__AVEENG_L.obj");
    // Model maglev("../src/assets/models/train/AnyConv.com__AVEENG_L.obj", texture_images);

    // Model field("../src/assets/models/field/mount.blend1.obj");

    // Skybox sb("../src/assets/skyboxes/skybox1.jpg");
    // Hut b;

    // Terrain tr("../src/assets/models/terrain/mount.blend1.obj");)
    Terrain tr(
        "../src/assets/models/field/mount.blend1.obj",
        "../src/assets/models/field",
        "../src/shaders/terrain.vert",
        "../src/shaders/terrain.frag"
    );


    do{
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 viewMatrix = camera->getViewMatrix();
        glm::mat4 projectionMatrix = camera->getProjectionMatrix();

        glm::mat4 vp = projectionMatrix * viewMatrix;

        glm::vec3 cameraPosition = camera->getCameraPosition();

        glm::mat4 skyBoxVP = projectionMatrix * glm::mat4(glm::mat3(viewMatrix));

        // sb.render(vp, cameraPosition);
        // b.render(vp);

        // field.render(vp);
        tr.render(vp);
        // maglev.render(vp);


        glfwSwapBuffers(window);
        glfwPollEvents();
    } while(!glfwWindowShouldClose(window));


    // Clear all the buffers that we created
    delete camera;


    glfwTerminate();

    return 0;
}