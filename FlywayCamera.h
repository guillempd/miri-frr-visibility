#ifndef _FLYWAY_CAMERA_INCLUDE
#define _FLYWAY_CAMERA_INCLUDE
#include "Camera.h"

class FlywayCamera : public ICamera
{
public:
    FlywayCamera();
    ~FlywayCamera() override;
    void init() override;
    void update(float deltaTime) override;
    void rotateCamera(float xRotation, float yRotation) override;
private:
    void moveForward(float input, float deltaTime);
    void moveRight(float input, float deltaTime);
    void moveUp(float input, float deltaTime);
    void updateLookDirection();
private:
    float theta;
    float phi;
};

#endif // _FLYWAY_CAMERA_INCLUDE