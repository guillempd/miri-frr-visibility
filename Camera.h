#ifndef _CAMERA_INCLUDE
#define _CAMERA_INCLUDE

#include <glm/glm.hpp>

#include <array>

struct Frustum
{
    std::array<glm::vec4, 6> planes;
};

// Camera contains the properies of the camera the scene is using
// It is responsible for computing the associated GL matrices

class Camera
{

public:
    Camera();
    ~Camera();
    void init();
    void update(float deltaTime);
    void resizeCameraViewport(int width, int height);
    void rotateCamera(float xRotation, float yRotation);
    void zoomCamera(float distDelta);
    const glm::mat4 &getProjectionMatrix() const;
    const glm::mat4 &getViewMatrix() const;
    const Frustum &getFrustum() const;
    const glm::vec3 &getPosition() const {return position;}
private:
    void moveForward(float input, float deltaTime);
    void moveRight(float input, float deltaTime);
    void moveUp(float input, float deltaTime);
    void updateViewMatrix();
    void updateLookDirection();
    void updateFrustum();
private:
    // OpenGL matrices
    glm::mat4 view;
    glm::mat4 projection;

    // Camera positioning
    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 lookDirection;
    float theta;
    float phi;

    // Camera movement
    float speed;
    float sensitivity;

    // Camera intrinsics
    float near;
    float far;
    float fov;
    float ar;

    // Others
    Frustum frustum;
};

#endif // _CAMERA_INCLUDE
