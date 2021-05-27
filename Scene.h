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
    int render();

    Camera &getCamera();

    void switchPolygonMode();

private:
    void initShaders();

    // Scene rendering algorithms
    int renderBasic();
    int renderFrustumCulling();
    int renderOcclusionCulling();
    int renderUltimate();

    // Objects rendering
    void render(const glm::ivec2 &position);
    void renderBoundingBox(const glm::ivec2 &position, bool wireframe);
    void renderFloor();

    bool insideFrustum(const glm::ivec2 &gridPosition) const;

    static glm::vec3 worldPosition(const glm::ivec2 &gridPosition);

private:
    Camera camera;
    TriangleMesh *mesh;
    TriangleMesh cube;
    TriangleMesh floor;
    ShaderProgram basicProgram;
    float currentTime;
    glm::mat4 floorModel;

    bool bPolygonFill;

    // Scene rendering data
    bool debug;
    bool frustumCulling;
    bool occlusionCulling;
    int n;

    // Occlusion culling data
    std::queue<Query> previousFrameQueries;
    std::vector<std::vector<Entity>> entities;
    QueryPool queryPool;
};

#endif // _SCENE_INCLUDE
