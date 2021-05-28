#include "PathCamera.h"

#include <fstream>

PathCamera::PathCamera(const std::string &path)
{
    ICamera::init();
    time = 0;

    std::ifstream fin(path);
    if (!fin.is_open()) return;

    fin >> n;

    pathPositions.resize(n);
    for (int i = 0; i <= n; ++i) {
        fin >> pathPositions[i].x >> pathPositions[i].y >> pathPositions[i].z;
    }

    pathLookDirections.resize(n);
    for (int i = 0; i <= n; ++i) {
        fin >> pathLookDirections[i].x >> pathLookDirections[i].y >> pathLookDirections[i].z;
    }
    
}

PathCamera::~PathCamera()
{

}

void PathCamera::init()
{

}

bool PathCamera::update(int deltaTime)
{
    time += deltaTime;
    int index = static_cast<float>(time)/1000.0f;
    float t = (time - index*1000)/1000.0f;
    if (index < n) {
        glm::vec3 position_i = pathPositions[index];
        glm::vec3 position_i_next = pathPositions[index+1];
        position = (1-t)*position_i + t*position_i_next;

        glm::vec3 lookDirection_i = pathLookDirections[index];
        glm::vec3 lookDirection_i_next = pathLookDirections[index+1];
        lookDirection = (1-t)*lookDirection_i + t*lookDirection_i_next;

        updateViewMatrix();
        return true;
    }
    else return false;
}