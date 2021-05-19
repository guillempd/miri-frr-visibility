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
    void render(int n);

    Camera &getCamera();

    void switchPolygonMode();

private:
    void initShaders();
    void computeModelViewMatrix();

    void render(int i, int j, const glm::mat4 &view, const glm::mat4 &projection);
    void renderBoundingBox();
    bool insideFrustum(const glm::mat4 &model) const;
    // bool insideFrustum(const glm::vec3 &point, const glm::mat4 &model, const glm::mat4 &view, const glm::mat4 &projection) const;

private:
    Camera camera;
    TriangleMesh *mesh;
    TriangleMesh cube;
    ShaderProgram basicProgram;
    float currentTime;

    bool bPolygonFill;
};

#endif // _SCENE_INCLUDE
