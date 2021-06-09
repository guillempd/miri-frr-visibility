#include "Camera.h"
#include "Application.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <iostream>
#include <fstream>

Camera::Camera()
{

}

void Camera::init()
{
    position = glm::vec3(0.0f, 0.0f, 3.0f);
    forward = glm::vec3(0.0f, 0.0f, -1.0f);
    right = glm::vec3(1.0f, 0.0f, 0.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    lookDirection = glm::vec3(0.0f, 0.0f, -1.0f);

    speed = 0.005f;
    sensitivity = 0.05f;

    near = 0.01f;
    far = 100.0f;
    fov = 60.f / 180.f * glm::pi<float>();
    
    int width = Application::instance().getWidth();
    int height = Application::instance().getHeight();
    resizeCameraViewport(width, height);

    updateViewMatrix();
}

void Camera::update(int deltaTime)
{
    if (replayMode) {
        replayTime += deltaTime;
        int index = replayTime/250;
        float t = static_cast<float>(replayTime - index*250)/250.0f;
        if (index < replayCheckpoints) {
            glm::vec3 position_i = replayPositions[index];
            glm::vec3 position_i_next = replayPositions[index+1];
            position = (1-t)*position_i + t*position_i_next;

            glm::vec3 lookDirection_i = replayLookDirections[index];
            glm::vec3 lookDirection_i_next = replayLookDirections[index+1];
            lookDirection = glm::normalize((1-t)*lookDirection_i + t*lookDirection_i_next);
            right = glm::normalize(glm::cross(lookDirection, up));
        }
        else endReplay();
    }

    else {
        if (Application::instance().getKey('w')) moveForward(1.0f, deltaTime);
        if (Application::instance().getKey('s')) moveForward(-1.0f, deltaTime);
        if (Application::instance().getKey('a')) moveRight(-1.0f, deltaTime);
        if (Application::instance().getKey('d')) moveRight(1.0f, deltaTime);
        if (Application::instance().getKey('q')) moveUp(-1.0f, deltaTime);
        if (Application::instance().getKey('e')) moveUp(1.0f, deltaTime);
    }

    updateViewMatrix();

    if (!recordMode) return;

    recordTimeSinceLastCheckpoint += deltaTime;
    if (recordTimeSinceLastCheckpoint >= 250) {
        recordTimeSinceLastCheckpoint = 0;
        recordPositions.push_back(position);
        recordLookDirections.push_back(lookDirection);
        if (!recordCheckpoints) endRecording();
        else --recordCheckpoints;
    }
}

void Camera::moveForward(float input, int deltaTime)
{
    position += (input * speed * deltaTime) * forward;
}

void Camera::moveUp(float input, int deltaTime)
{
    position += (input * speed * deltaTime) * up;
}

void Camera::moveRight(float input, int deltaTime)
{
    position += (input * speed * deltaTime) * right;
}

void Camera::beginRecording(const std::string &filePath, int duration)
{
    recordMode = true;
    recordFilePath = filePath;
    recordCheckpoints = 4 * duration;
    recordTimeSinceLastCheckpoint = 250;
    recordPositions.reserve(recordCheckpoints + 1);
    recordLookDirections.reserve(recordCheckpoints + 1);
}

void Camera::endRecording()
{
    std::ofstream fout(recordFilePath);
    if (fout.is_open()) {
        fout << recordPositions.size() << '\n';

        for (const auto &position : recordPositions) {
            fout << position.x << ' ' << position.y << ' ' << position.z << '\n';
        }

        fout << '\n';

        for (const auto &lookDirection : recordLookDirections) {
            fout << lookDirection.x << ' ' << lookDirection.y << ' ' << lookDirection.z << '\n';
        }
    }

    recordMode = false;
    recordPositions.clear();
    recordLookDirections.clear();
}

int Camera::beginReplay(const std::string &filePath)
{
    replayTime = 0;

    std::ifstream fin(filePath);
    if (!fin.is_open()) return 0;

    fin >> replayCheckpoints;

    replayPositions.resize(replayCheckpoints);
    for (int i = 0; i < replayCheckpoints; ++i) {
        fin >> replayPositions[i].x >> replayPositions[i].y >> replayPositions[i].z;
    }

    replayLookDirections.resize(replayCheckpoints);
    for (int i = 0; i < replayCheckpoints; ++i) {
        fin >> replayLookDirections[i].x >> replayLookDirections[i].y >> replayLookDirections[i].z;
    }

    replayMode = true;

    return replayCheckpoints - 1;
}

void Camera::endReplay()
{
    replayMode = false;
    replayPositions.clear();
    replayLookDirections.clear();
}

void Camera::resizeCameraViewport(int width, int height)
{
    if (width == 0 || height == 0) return;
    ar = static_cast<float>(width)/static_cast<float>(height);
    projection = glm::perspective(fov, ar, near, far);
    updateFrustum();
}

void Camera::rotateCamera(float xRotation, float yRotation)
{
    if (replayMode) return;

    theta += xRotation * sensitivity;
    phi += yRotation * sensitivity;
    phi = glm::clamp(phi, -(glm::half_pi<float>() - 0.1f), glm::half_pi<float>() - 0.1f);
    lookDirection = glm::vec3(glm::cos(phi) * glm::cos(theta) ,glm::sin(phi), -glm::cos(phi) * glm::sin(theta));
    forward = glm::normalize(glm::vec3(lookDirection.x, 0.0f, lookDirection.z));
    right = glm::cross(forward, up);
    updateViewMatrix();
}

void Camera::zoomCamera(float distDelta)
{

}

void Camera::updateViewMatrix()
{
    view = glm::lookAt(position, position + lookDirection, up);
    updateFrustum();
}

// Computes the planes of the frustum in world space coordinates
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
    frustum.planes[4] = glm::vec4(-lookDirection, -glm::dot(position + lookDirection * near, -lookDirection));
    frustum.planes[5] = glm::vec4(lookDirection, -glm::dot(position + lookDirection * far, lookDirection));
}
