#include "FlywayCamera.h"
#include "Application.h"

#include "glm/glm.hpp"
#include "glm/ext.hpp"

FlywayCamera::FlywayCamera()
{

}

FlywayCamera::~FlywayCamera()
{

}

void FlywayCamera::init()
{
    ICamera::init();
    theta = 0;
    phi = 0;
}

bool FlywayCamera::update(int deltaTime)
{
    ICamera::update(deltaTime);
    
    if (Application::instance().getKey('w')) moveForward(1.0f, deltaTime);
    if (Application::instance().getKey('s')) moveForward(-1.0f, deltaTime);
    if (Application::instance().getKey('a')) moveRight(-1.0f, deltaTime);
    if (Application::instance().getKey('d')) moveRight(1.0f, deltaTime);
    if (Application::instance().getKey('q')) moveUp(-1.0f, deltaTime);
    if (Application::instance().getKey('e')) moveUp(1.0f, deltaTime);
    updateViewMatrix();
    return true;
}

void FlywayCamera::moveForward(float input, int deltaTime)
{
    position += (input * speed * deltaTime) * forward;
}

void FlywayCamera::moveUp(float input, int deltaTime)
{
    position += (input * speed * deltaTime) * up;
}

void FlywayCamera::moveRight(float input, int deltaTime)
{
    position += (input * speed * deltaTime) * right;
}

void FlywayCamera::rotateCamera(float xRotation, float yRotation)
{
    theta += xRotation * sensitivity;
    phi += yRotation * sensitivity;
    phi = glm::clamp(phi, -(glm::half_pi<float>() - 0.1f), glm::half_pi<float>() - 0.1f);
    updateLookDirection();
}

void FlywayCamera::updateLookDirection()
{
    lookDirection = glm::vec3(glm::cos(phi) * glm::cos(theta) ,glm::sin(phi), -glm::cos(phi) * glm::sin(theta));
    forward = glm::normalize(glm::vec3(lookDirection.x, 0.0f, lookDirection.z));
    right = glm::cross(forward, up);
    updateViewMatrix();
}
