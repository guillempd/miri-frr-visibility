#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE

#include "Camera.h"
#include "Query.h"
#include "QueryPool.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"
#include "Quadtree.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <queue>
#include <stack>
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
    // Frustum culling implementation
    bool insideFrustum(const AABB &aabb) const;
    bool insideFrustum(const glm::ivec2 &gridPosition) const;
    
    // Scene rendering algorithms
    int renderBasic();
    int renderStopAndWait();
    int renderAdvanced();
    int renderCHC();

    // Objects rendering
    void render(const glm::ivec2 &gridPosition);
    void renderBoundingBox(const glm::ivec2 &gridPosition, bool wireframe);
    void renderBoundingBox(const glm::mat4 &model, bool wireframe);
    void renderFloor();
    static glm::vec3 worldPosition(const glm::ivec2 &gridPosition);

    // CHC implementation functions
    void buildSceneHierarchy();
    void buildSceneHierarchy(QuadtreeNodeIndex nodeIndex);
    void pullUpVisibility(QuadtreeNodeIndex nodeIndex);
    void addChildren(QuadtreeNodeIndex nodeIndex, std::stack<QuadtreeNodeIndex> &nodes);
    int render(QuadtreeNodeIndex nodeIndex);
    void renderBoundingBox(QuadtreeNodeIndex nodeIndex, bool wireframe);
    Query renderWithQuery(QuadtreeNodeIndex nodeIndex);
    Query issueQuery(QuadtreeNodeIndex nodeIndex);
    void renderSceneHierarchy(QuadtreeNodeIndex nodeIndex);

    // Others
    void initShaders();
    float distanceToCamera(const glm::ivec2 &gridPosition);

private:
    // Scene elements
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
    unsigned int currentFrame;

    enum OcclusionQueriesAlgorithm
    {
        NONE,
        STOP_AND_WAIT,
        ADVANCED,
        CHC
    };

    using QueryInfo = std::pair<Query,glm::ivec2>;
    using DistancePosition = std::pair<float,glm::ivec2>;

    // Occlusion culling data (Advanced)
    QueryPool queryPool;
    std::queue<QueryInfo> previousFrameQueries;
    std::unordered_set<glm::ivec2> PVS;

    // Occlusion culling data (CHC)
    Quadtree sceneHierarchy;
    int maxDepth;
    std::vector<std::vector<bool>> alreadyRendered;

};

#endif // _SCENE_INCLUDE
