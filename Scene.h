#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE

#include <glm/glm.hpp>
#include "Camera.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"

// Scene contains all the entities of our game.
// It is responsible for updating and render them.

class Scene
{

public:
    Scene();
    ~Scene();

    void init();
    bool loadMesh(const char *filename);
    void update(int deltaTime);
    int render(int n);

    Camera &getCamera();

    void switchPolygonMode();

private:
    void initShaders();
    void computeModelViewMatrix();

    bool render(int i, int j);
    bool insideFrustum(const glm::mat4 &model) const;

private:
    Camera camera;
    TriangleMesh *mesh;
    TriangleMesh cube;
    ShaderProgram basicProgram;
    float currentTime;

    bool bPolygonFill;
};

#endif // _SCENE_INCLUDE
