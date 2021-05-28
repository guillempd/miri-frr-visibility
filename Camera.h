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
class ICamera
{

public:
    ICamera();
    virtual ~ICamera() = default; // default or delete?
    virtual void init();
    virtual void update(float deltaTime) = 0;
    virtual void rotateCamera(float xRotation, float yRotation);
    void resizeCameraViewport(int width, int height);
    void zoomCamera(float distDelta);
    const glm::vec3 &getPosition() const {return position;}
    const glm::mat4 &getViewMatrix() const {return view;}
    const glm::mat4 &getProjectionMatrix() const {return projection;}
    const Frustum &getFrustum() const {return frustum;}
protected:
    void updateViewMatrix();
    void updateFrustum();
protected:
    // OpenGL matrices
    glm::mat4 view;
    glm::mat4 projection;

    // Camera positioning
    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 lookDirection;

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
