#include <GL/glew.h>
#include <GL/glut.h>

#include <string>
#include "imgui.h"

#include "Application.h"

void Application::init()
{
    bPlay = true;
    glClearColor(1.f, 1.f, 1.f, 1.0f);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    scene.init();

    for (unsigned int i = 0; i < 256; i++)
    {
        keys[i] = false;
        specialKeys[i] = false;
    }
    mouseButtons[0] = false;
    mouseButtons[1] = false;
    lastMousePos = glm::ivec2(-1, -1);

    mouseSensitivity = 0.05f;

    time = 0;
    frames = 0;
    fps = 0.0f;
}

bool Application::loadMesh(const char *filename)
{
    return scene.loadMesh(filename);
}

bool Application::update(int deltaTime)
{
    scene.update(deltaTime);
    updateFrameRate(deltaTime);
    return bPlay;
}

void Application::updateFrameRate(int deltaTime)
{
    frames += 1;
    time += deltaTime;
    if (time >= SAMPLE_TIME) {
        fps = static_cast<float>(frames)/static_cast<float>(time) * 1000;
        time = 0;
        frames = 0;
    }
}

void Application::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int rendered = scene.render();

    if (ImGui::Begin("Performance Statistics")) {
        ImGui::Text((std::to_string(fps) + std::string("fps")).c_str());
        ImGui::Text(("Rendered copies: " + std::to_string(rendered)).c_str());
    }
    ImGui::End();
    
}

void Application::resize(int width, int height)
{
    this->width = width;
    this->height = height;
    glViewport(0, 0, width, height);
    scene.getCamera().resizeCameraViewport(width, height);
}

void Application::keyPressed(int key)
{
    if (key == 27) // Escape code
        bPlay = false;
    keys[key] = true;
}

void Application::keyReleased(int key)
{
    keys[key] = false;
}

void Application::specialKeyPressed(int key)
{
    specialKeys[key] = true;
}

void Application::specialKeyReleased(int key)
{
    specialKeys[key] = false;
}

void Application::mouseMove(int x, int y)
{
    // Rotation
    if (lastMousePos.x != -1)
        scene.getCamera().rotateCamera(-mouseSensitivity * x, -mouseSensitivity * y);

    // Zoom
    if (mouseButtons[1] && lastMousePos.x != -1)
        scene.getCamera().zoomCamera(0.01f * (y - lastMousePos.y));

    lastMousePos = glm::ivec2(x, y);
}

void Application::mousePress(int button)
{
    mouseButtons[button] = true;
}

void Application::mouseRelease(int button)
{
    mouseButtons[button] = false;
    if (!mouseButtons[0] && !mouseButtons[1])
        lastMousePos = glm::ivec2(-1, -1);
}

bool Application::getKey(int key) const
{
    return keys[key];
}

bool Application::getSpecialKey(int key) const
{
    return specialKeys[key];
}

int Application::getWidth() const
{
    return width;
}

int Application::getHeight() const
{
    return height;
}
