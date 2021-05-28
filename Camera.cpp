#include "Camera.h"
#include "Application.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <iostream>
#include <fstream>

ICamera::ICamera()
{

}

void ICamera::init()
{
    position = glm::vec3(0.0f, 0.0f, 3.0f);
    forward = glm::vec3(0.0f, 0.0f, -1.0f);
    right = glm::vec3(1.0f, 0.0f, 0.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    lookDirection = glm::vec3(0.0f, 0.0f, -1.0f);

    speed = 0.01f;
    sensitivity = 0.1f;

    near = 0.01f;
    far = 100.0f;
    fov = 60.f / 180.f * glm::pi<float>();
    
    resizeCameraViewport(16, 9);

    updateViewMatrix();
}

bool ICamera::update(int deltaTime)
{
    timeSinceLastCheckpoint += deltaTime;
    if (timeSinceLastCheckpoint >= 1000) {
        timeSinceLastCheckpoint = 0;
        recordingPositions.push_back(position);
        recordingLookDirections.push_back(lookDirection);
        if (!checkpointsLeft) savePath();
        else --checkpointsLeft;
    }
    return true;
}

void ICamera::savePath()
{
    std::ofstream fout(recordingPath);
    // if (!fout.is_open()) return;

    int n = recordingPositions.size();
    fout << n << '\n';

    for (const auto &position : recordingPositions) {
        fout << position.x << ' ' << position.y << ' ' << position.z << '\n';
    }

    fout << '\n';

    for (const auto &lookDirection : recordingLookDirections) {
        fout << lookDirection.x << ' ' << lookDirection.y << ' ' << lookDirection.z << '\n';
    }
}

void ICamera::resizeCameraViewport(int width, int height)
{
    ar = static_cast<float>(width)/static_cast<float>(height);
    projection = glm::perspective(fov, ar, near, far);
    updateFrustum();
}

void ICamera::rotateCamera(float xRotation, float yRotation)
{
    
}

void ICamera::zoomCamera(float distDelta)
{

}

void ICamera::recordPath(const std::string &path, int duration)
{
    recordingPath = path;
    checkpointsLeft = duration;
    timeSinceLastCheckpoint = 1000;
    recordingPositions.clear();
    recordingPositions.reserve(checkpointsLeft + 1);
    recordingLookDirections.clear();
    recordingLookDirections.reserve(checkpointsLeft + 1);
}

void ICamera::updateViewMatrix()
{
    view = glm::lookAt(position, position + lookDirection, up);
    updateFrustum();
}

// Computes the planes of the frustum in world space coordinates
void ICamera::updateFrustum()
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
    frustum.planes[4] = glm::vec4(-lookDirection, -glm::dot(position + lookDirection * near, -lookDirection));
    frustum.planes[5] = glm::vec4(lookDirection, -glm::dot(position + lookDirection * far, lookDirection));
}
