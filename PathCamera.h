#ifndef _PATH_CAMERA_INCLUDE
#define _PATH_CAMERA_INCLUDE
#include "Camera.h"

#include "glm/glm.hpp"

#include <string>
#include <vector>

class PathCamera : public ICamera
{
public:
    PathCamera(const std::string &path);
    void init() override;
    bool update(int deltaTime) override;
private:
    std::vector<glm::vec3> pathPositions;
    std::vector<glm::vec3> pathLookDirections;
    float time;
    int n;
};

#endif // _PATH_CAMERA_INCLUDE