#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace glm;

class Camera{
    vec3 cameraPosition = vec3(0, 5, 0);
    vec3 cameraFront    = vec3(0, 5, -1);
    const vec3 cameraUp       = vec3(0, 1, 0);

    vec3 target;

    GLfloat yawAngle;

    const GLfloat yawSensitivity = 2.5;
    const GLfloat speed = 10.0f;

    float32 FoV     = 90.0f;
    float32 zNear   = 0.1f;
    float32 zFar    = 1000000.0f;

    mat4 projectionMatrix;

    private:
        vec3 calculateCameraFront() {
            glm::vec3 front;
            front.x = cos(radians(yawAngle));
            front.y = 0.0f;
            front.z = sin(radians(yawAngle));
            return glm::normalize(front);
        }

        void computeYawAngle(){
            // compute the yaw angle
            double angleRadians = atan2(cameraFront.z, cameraFront.x);
            this->yawAngle = angleRadians * 180 / M_PI;
        }

    public:
        Camera(){
            projectionMatrix = glm::perspective(radians(FoV), 16.0f/9.0f, zNear, zFar);
        }

        mat4 getProjectionMatrix(){
            return projectionMatrix;
        }

        mat4 getViewMatrix(){
            // Also change the camera angle
            vec3 cameraFront = calculateCameraFront();

            target = cameraPosition + cameraFront;

            computeYawAngle();

            return lookAt(cameraPosition, target, cameraUp);
        }

        vec3 getCameraPosition(){
            return this->target;
        }

        void resetCamera(){
            cameraPosition  = vec3(0, 5, 0);
            cameraFront     = vec3(0, 5, -1);
            yawAngle        = 0;
        }

        void lookRight(){
            GLfloat angle = radians(yawSensitivity);
            mat3 rotationMatrix = mat3(cos(angle), 0, sin(angle),0, 1, 0,-sin(angle), 0, cos(angle));
            cameraFront = rotationMatrix * cameraFront;
        }

        void lookLeft(){
            GLfloat angle = -radians(yawSensitivity);
            mat3 rotationMatrix = mat3(cos(angle), 0, sin(angle),0, 1, 0,-sin(angle), 0, cos(angle));
            cameraFront = rotationMatrix * cameraFront;
        }

        void moveForward(){
            cameraPosition += speed * calculateCameraFront();
        }

        void moveBackward(){
            cameraPosition -= speed * calculateCameraFront();
        }

        void moveRight(){
            cameraPosition += speed * normalize(cross(cameraFront, cameraUp));
        }

        void moveLeft(){
            cameraPosition -= speed * normalize(cross(cameraFront, cameraUp));
        }

        ~Camera(){
            std::cout << "Deleting the camera object." << std::endl;
        }
};