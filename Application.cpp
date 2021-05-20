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

    frameCount = 0;
    accumulatedDeltaTime = 0;
    frameRate = 0.0f;

    copies = 1;

    mouseSensitivity = 0.05f;
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
    ++frameCount;
    accumulatedDeltaTime += deltaTime;
    if (frameCount == FRAMES_TO_COUNT)
    {
        frameRate = (1000.0f * FRAMES_TO_COUNT)/accumulatedDeltaTime;
        frameCount = 0;
        accumulatedDeltaTime = 0;
    }
}

void Application::render()
{
    if (ImGui::Begin("Scene Size")) {
        ImGui::PushButtonRepeat(true);
        if (ImGui::ArrowButton("##left", ImGuiDir_Left)) copies = (--copies > 0 ? copies : 0);
        ImGui::SameLine();
        if (ImGui::ArrowButton("##right", ImGuiDir_Right)) copies += 1;
        ImGui::PopButtonRepeat();
        ImGui::SameLine();
        ImGui::Text("%d", copies);
    }
    ImGui::End();


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int rendered = scene.render(copies);

    if (ImGui::Begin("Performance Statistics")) {
        ImGui::Text((std::to_string(frameRate) + std::string("fps")).c_str());
        ImGui::Text(("Rendered copies: " + std::to_string(rendered)).c_str());
    }
    ImGui::End();
    
}

void Application::resize(int width, int height)
{
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
    if (key == GLUT_KEY_F1)
        scene.switchPolygonMode();
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
