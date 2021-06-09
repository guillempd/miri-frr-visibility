#include "Application.h"

#include "imgui.h"

#include <GL/glut.h>
#include <GL/glew.h>

#include <fstream>

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

    if (recordMode) {
        recordTimeSinceLastCheckpoint += deltaTime;
        if (recordTimeSinceLastCheckpoint >= 250) {
            recordTime.push_back(recordTimeSinceLastCheckpoint);
            recordFps.push_back(fps);
            recordTimeSinceLastCheckpoint = 0;
            if (!recordCheckpoints) endRecordFps();
            else --recordCheckpoints;
        }
    }
}

void Application::beginRecordFps(const std::string &filePath, int duration)
{
    recordMode = true;
    recordFilePath = filePath;
    recordCheckpoints = duration;
    recordTimeSinceLastCheckpoint = 250;
    recordTime.reserve(recordCheckpoints + 1);
    recordFps.reserve(recordCheckpoints + 1);
}

void Application::endRecordFps()
{
    recordMode = false;
    
    std::ofstream fout(recordFilePath);
    if (fout.is_open()) {
        int n = recordFps.size();
        int accumulatedTime = 0;
        fout << "0.0 " << recordFps[0] << '\n';
        for (int i = 1; i < n; ++i) {
            accumulatedTime += recordTime[i];
            fout << accumulatedTime/1000.0f << ' ' << recordFps[i] << '\n';
        }
    }
    recordTime.clear();
    recordFps.clear();
}

void Application::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int rendered = scene.render();

    if (ImGui::Begin("Performance Statistics")) {
        ImGui::Text("%g fps", fps);
        ImGui::Text("Rendered copies: %i", rendered);
    }
    ImGui::End();

    if (ImGui::Begin("Record Path")) {
        ImGui::InputText("Output Path File", outputPathBuff, IM_ARRAYSIZE(outputPathBuff));
        ImGui::SliderInt("Duration", &pathDuration, 1, 60);
        if (ImGui::Button("Record")) {
            scene.getCamera().beginRecording(outputPathBuff, pathDuration);
        }
    }
    ImGui::End();

    if (ImGui::Begin("Replay Path")) {
        ImGui::InputText("Input Path File", inputPathBuff, IM_ARRAYSIZE(inputPathBuff));
        ImGui::InputText("Output FPS File", outputFpsBuff, IM_ARRAYSIZE(outputFpsBuff));
        if (ImGui::Button("Replay")) {
            int duration = scene.getCamera().beginReplay(inputPathBuff);
            beginRecordFps(outputFpsBuff, duration);
        }
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
