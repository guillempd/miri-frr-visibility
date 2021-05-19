#include "Camera.h"
#include "Application.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <iostream>

Camera::Camera()
{

}

Camera::~Camera()
{

}

void Camera::init()
{
    position = glm::vec3(0.0f, 0.0f, 3.0f);
    forward = glm::vec3(0.0f, 0.0f, -1.0f);
    right = glm::vec3(1.0f, 0.0f, 0.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    lookDirection = glm::vec3(0.0f, 0.0f, -1.0f);
    theta = glm::half_pi<float>();
    phi = 0.0f;

    speed = 0.01f;
    sensitivity = 0.1f;

    near = 0.01f;
    far = 100.0f;
    fov = 60.f / 180.f * glm::pi<float>();
    ar = 1.0f;

    updateViewMatrix();
}

void Camera::update(float deltaTime)
{
    if (Application::instance().getKey('w')) moveForward(1.0f, deltaTime);
    if (Application::instance().getKey('s')) moveForward(-1.0f, deltaTime);
    if (Application::instance().getKey('a')) moveRight(-1.0f, deltaTime);
    if (Application::instance().getKey('d')) moveRight(1.0f, deltaTime);
    if (Application::instance().getKey('q')) moveUp(-1.0f, deltaTime);
    if (Application::instance().getKey('e')) moveUp(1.0f, deltaTime);
    updateViewMatrix();
}

void Camera::resizeCameraViewport(int width, int height)
{
    ar = static_cast<float>(width)/static_cast<float>(height);
    projection = glm::perspective(fov, ar, near, far);
    updateFrustum();
}

void Camera::rotateCamera(float xRotation, float yRotation)
{
    theta += xRotation * sensitivity;
    phi += yRotation * sensitivity;
    phi = glm::clamp(phi, -(glm::half_pi<float>() - 0.1f), glm::half_pi<float>() - 0.1f);
    updateLookDirection();
}

// TODO: Implement this changing fov
void Camera::zoomCamera(float distDelta)
{

}

const glm::mat4 &Camera::getProjectionMatrix() const
{
    return projection;
}

const glm::mat4 &Camera::getViewMatrix() const
{
    return view;
}

const Frustum &Camera::getFrustum() const
{
    return frustum;
}

void Camera::moveForward(float input, float deltaTime)
{
    position += input * forward * speed * deltaTime;
}

void Camera::moveUp(float input, float deltaTime)
{
    position += input * up * speed * deltaTime;
}

void Camera::moveRight(float input, float deltaTime)
{
    position += input * right * speed * deltaTime;
}

void Camera::updateLookDirection()
{
    lookDirection = glm::vec3(glm::cos(phi) * glm::cos(theta) ,glm::sin(phi), -glm::cos(phi) * glm::sin(theta));
    forward = glm::normalize(glm::vec3(lookDirection.x, 0.0f, lookDirection.z));
    right = glm::cross(forward, up);
    updateViewMatrix();
}

void Camera::updateViewMatrix()
{
    view = glm::lookAt(position, position + lookDirection, up);
    updateFrustum();
}

void Camera::updateFrustum()
{
    float xOffset = ar * near * glm::tan(fov/2);
    float yOffset = near * glm::tan(fov/2);
    float zOffset = -near;

    glm::vec3 tl(-xOffset, +yOffset, +zOffset);
    glm::vec3 tr(+xOffset, +yOffset, +zOffset);
    glm::vec3 bl(-xOffset, -yOffset, +zOffset);
    glm::vec3 br(+xOffset, -yOffset, +zOffset);

    glm::vec3 t = glm::normalize(glm::cross(tr, tl));
    glm::vec3 b = glm::normalize(glm::cross(bl, br));
    glm::vec3 l = glm::normalize(glm::cross(tl, bl));
    glm::vec3 r = glm::normalize(glm::cross(br, tr));

    glm::mat3 transform(right, glm::cross(-lookDirection, right), -lookDirection);

    t = transform * t;
    b = transform * b;
    l = transform * l;
    r = transform * r;

    frustum.planes[0] = glm::vec4(t, -glm::dot(position, t));
    frustum.planes[1] = glm::vec4(b, -glm::dot(position, b));
    frustum.planes[2] = glm::vec4(l, -glm::dot(position, l));
    frustum.planes[3] = glm::vec4(r, -glm::dot(position, r));
    frustum.planes[4] = glm::vec4(-lookDirection, -glm::dot(-lookDirection, position + lookDirection * near));
    frustum.planes[5] = glm::vec4(lookDirection, -glm::dot(lookDirection, position + lookDirection * far));
}
