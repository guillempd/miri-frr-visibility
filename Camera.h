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
class Camera
{

public:
    Camera();
    void init();
    void update(int deltaTime);
    void rotateCamera(float xRotation, float yRotation);
    void resizeCameraViewport(int width, int height);
    void zoomCamera(float distDelta);

    void beginRecording(const std::string &filePath, int duration);
    void endRecording();
    void beginReplay(const std::string &filePath);
    void endReplay();

    const glm::vec3 &getPosition() const {return position;}
    const glm::mat4 &getViewMatrix() const {return view;}
    const glm::mat4 &getProjectionMatrix() const {return projection;}
    const Frustum &getFrustum() const {return frustum;}
private:
    void moveForward(float input, int deltaTime);
    void moveRight(float input, int deltaTime);
    void moveUp(float input, int deltaTime);
    // void savePath();
    void updateViewMatrix();
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

    // Camera record
    bool recordMode;
    std::string recordFilePath;
    std::vector<glm::vec3> recordPositions;
    std::vector<glm::vec3> recordLookDirections;
    int recordCheckpoints;
    int recordTimeSinceLastCheckpoint;

    // Camera replay
    bool replayMode;
    std::vector<glm::vec3> replayPositions;
    std::vector<glm::vec3> replayLookDirections;
    int replayCheckpoints;
    int replayTime;

    // Others
    Frustum frustum;
};

#endif // _CAMERA_INCLUDE
