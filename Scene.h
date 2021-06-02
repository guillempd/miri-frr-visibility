#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE

#include "Camera.h"
#include "Query.h"
#include "QueryPool.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"

#include <glm/glm.hpp>

#include <queue>

struct Entity
{
    bool wasVisible;
};

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
    int render();

    Camera &getCamera() {return camera;}

private:
    // Scene rendering algorithms
    int renderBasic();
    int renderFrustumCulling();
    int renderOcclusionCulling();
    int renderUltimate();
    bool insideFrustum(const glm::ivec2 &gridPosition) const;

    // Objects rendering
    void render(const glm::ivec2 &position);
    void renderBoundingBox(const glm::ivec2 &position, bool wireframe);
    void renderFloor();
    static glm::vec3 worldPosition(const glm::ivec2 &gridPosition);

    // Camera and Paths management
    void beginReplayCameraPath(const std::string &path);
    void endReplayCameraPath();
    void recordCameraPath(const std::string &path, int duration);

    // Others
    void initShaders();

private:
    Camera camera;
    TriangleMesh mesh;
    TriangleMesh cube;
    TriangleMesh floor;
    ShaderProgram basicProgram;
    glm::mat4 floorModel;

    // Scene rendering data
    bool debugMode;
    bool pathMode;
    bool frustumCulling;
    bool occlusionCulling;
    int n;

    // Path recording data
    char inputBuff[64];
    char outputBuff[64];

    // Occlusion culling data
    // std::queue<Query> previousFrameQueries;
    // std::vector<std::vector<Entity>> entities;
    // QueryPool queryPool;
};

#endif // _SCENE_INCLUDE
