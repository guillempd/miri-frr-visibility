#ifndef _FLYWAY_CAMERA_INCLUDE
#define _FLYWAY_CAMERA_INCLUDE
#include "Camera.h"

class FlywayCamera : public ICamera
{
public:
    FlywayCamera();
    void init() override;
    bool update(int deltaTime) override;
    void rotateCamera(float xRotation, float yRotation) override;
private:
    void moveForward(float input, int deltaTime);
    void moveRight(float input, int deltaTime);
    void moveUp(float input, int deltaTime);
    void updateLookDirection();
private:
    float theta;
    float phi;
};

#endif // _FLYWAY_CAMERA_INCLUDE