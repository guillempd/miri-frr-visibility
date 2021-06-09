#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE

#include "Camera.h"
#include "Query.h"
#include "QueryPool.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <queue>
#include <utility>
#include <unordered_set>

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
    int renderStopAndWait();
    int renderCHC();

    // Frustum culling implementation
    bool insideFrustum(const glm::ivec2 &gridPosition) const;

    // Objects rendering
    void render(const glm::ivec2 &position);
    void renderBoundingBox(const glm::ivec2 &position, bool wireframe);
    void renderFloor();
    static glm::vec3 worldPosition(const glm::ivec2 &gridPosition);

    // Others
    void initShaders();
    float distanceToCamera(const glm::ivec2 &gridPosition);

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
    int occlusionCulling;
    int n;

    enum OcclusionQueriesAlgorithm
    {
        NONE,
        STOP_AND_WAIT,
        CHC
    };

    using QueryInfo = std::pair<Query,glm::ivec2>;

    // Occlusion culling data
    QueryPool queryPool;
    std::queue<QueryInfo> previousFrameQueries;
    std::unordered_set<glm::ivec2> V; // TODO: Rename to PVS
    // std::vector<std::vector<Entity>> entities;
};

#endif // _SCENE_INCLUDE
