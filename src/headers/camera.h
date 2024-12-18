#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace glm;

class Camera{
    vec3 cameraPosition = vec3(0, 50, 0);
    vec3 cameraFront    = vec3(0, 50, -1);
    const vec3 cameraUp = vec3(0, 1, 0);

    vec3 target;

    GLfloat yawAngle;

    const GLfloat yawSensitivity = 1;
    const GLfloat speed = 10.0f;

    float32 FoV     = 45;
    float32 zNear   = 0.1f;
    float32 zFar    = 10000.0f;

    mat4 projectionMatrix;

    private:
        vec3 calculateCameraFront() {
            glm::vec3 front;
            front.x = cos(radians(yawAngle));
            front.y = 0.0;
            front.z = sin(radians(yawAngle));
            return front;
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

        mat4 getViewProjectionMatrix(){
            // Also change the camera angle
            vec3 cameraFront = calculateCameraFront();

            target = cameraPosition + cameraFront;
            // vec3 target = cameraFront + cameraFront ;

            computeYawAngle();

            return projectionMatrix * lookAt(cameraPosition, target, cameraUp);
        }

        vec3 getCameraPosition(){


            return this->target;
        }

        void resetCamera(){
            cameraPosition  = vec3(0, 50, 0);
            cameraFront     = vec3(0, 50, -1);
        }

        void lookRight(){
            GLfloat angle = radians(yawSensitivity);
            mat3 rotationMatrix = glm::mat3(cos(angle), 0, sin(angle),0, 1, 0,-sin(angle), 0, cos(angle));
            cameraFront = rotationMatrix * cameraFront;
        }

        void lookLeft(){
            GLfloat angle = -radians(yawSensitivity);
            mat3 rotationMatrix = glm::mat3(cos(angle), 0, sin(angle),0, 1, 0,-sin(angle), 0, cos(angle));
            cameraFront = rotationMatrix * cameraFront;
        }

        void moveForward(){


            vec3 shiftVector = vec3(speed * cos(yawAngle) - speed * sin(yawAngle), 0,  speed * sin(yawAngle) + speed * cos(yawAngle));

            cameraPosition += speed * calculateCameraFront();
        }

        void moveBackward(){
            vec3 shiftVector = vec3(speed * cos(yawAngle) - speed * sin(yawAngle), 0,  speed * sin(yawAngle) + speed * cos(yawAngle));


            cameraPosition -= speed * calculateCameraFront();
        }

        ~Camera(){

        }
};