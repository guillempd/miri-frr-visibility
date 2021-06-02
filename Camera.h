#ifndef _CAMERA_INCLUDE
#define _CAMERA_INCLUDE

#include <glm/glm.hpp>

#include <array>
#include <string>
#include <vector>

struct Frustum
{
    std::array<glm::vec4, 6> planes;
};

// Camera contains the properies of the camera the scene is using
// It is responsible for computing the associated GL matrices
class ICamera
{

public:
    virtual ~ICamera() = default;
    virtual void init();
    virtual bool update(int deltaTime);
    virtual void rotateCamera(float xRotation, float yRotation);
    void resizeCameraViewport(int width, int height);
    void zoomCamera(float distDelta);
    void recordPath(const std::string &path, int duration);
    const glm::vec3 &getPosition() const {return position;}
    const glm::mat4 &getViewMatrix() const {return view;}
    const glm::mat4 &getProjectionMatrix() const {return projection;}
    const Frustum &getFrustum() const {return frustum;}
protected:
    void savePath();
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

    // Camera recording
    bool recording;
    std::string recordingPath;
    std::vector<glm::vec3> recordingPositions;
    std::vector<glm::vec3> recordingLookDirections;
    int checkpointsLeft;
    int timeSinceLastCheckpoint;

    // Others
    Frustum frustum;
};

#endif // _CAMERA_INCLUDE
