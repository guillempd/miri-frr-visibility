#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE

#include "Camera.h"
#include "Query.h"
#include "QueryPool.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"

#include <glm/glm.hpp>

#include <queue>

// Scene contains all the entities of our game.
// It is responsible for updating and render them.


struct Entity
{
    bool wasVisible;
};

class Scene
{

public:
    Scene();
    ~Scene();

    void init();
    bool loadMesh(const char *filename);
    void update(int deltaTime);
    int render(int n, bool frustumCulling, bool occlusionCulling);

    Camera &getCamera();

    void switchPolygonMode();

private:
    void initShaders();
    void computeModelViewMatrix();

    bool render(int i, int j, bool frustumCulling, bool occlusionCulling);
    void render(glm::ivec2 position);
    void renderQueryBox(glm::ivec2 position);

    bool insideFrustum(const glm::mat4 &model) const;
    void computeVisibility();

    int renderWithOcclusionCulling();

private:
    Camera camera;
    TriangleMesh *mesh;
    TriangleMesh cube;
    ShaderProgram basicProgram;
    float currentTime;

    bool bPolygonFill;

    int n;

    // Occlusion culling data
    std::queue<Query> previousFrameQueries;
    std::vector<std::vector<Entity>> entities;
    QueryPool queryPool;
};

#endif // _SCENE_INCLUDE
