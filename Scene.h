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
    // Scene rendering algorithms
    int renderBasic();
    int renderStopAndWait();
    int renderAdvanced();
    int renderCHC();

    // Frustum culling implementation
    bool insideFrustum(const glm::ivec2 &gridPosition) const;

    // Objects rendering
    void render(const glm::ivec2 &position);
    void renderBoundingBox(const QuadtreeNode &node, bool wireframe);
    void renderBoundingBox(const glm::ivec2 &gridPosition, bool wireframe);
    void renderBoundingBox(const glm::mat4 &model, bool wireframe);
    void renderFloor();
    void renderNode();
    static glm::vec3 worldPosition(const glm::ivec2 &gridPosition);

    // CHC stuff
    void CHC_constructSceneHierarchy();
    void CHC_constructSceneHierarchy(QuadtreeNodeIndex nodeIndex, int depth);
    void CHC_pullUpVisibility(QuadtreeNodeIndex nodeIndex);
    void CHC_addChildren(QuadtreeNodeIndex nodeIndex, std::stack<QuadtreeNodeIndex> &nodes);
    int CHC_render(QuadtreeNodeIndex nodeIndex);
    void CHC_renderBoundingBox(QuadtreeNodeIndex nodeIndex, bool wireframe);
    Query CHC_renderWithQuery(QuadtreeNodeIndex nodeIndex);
    Query CHC_issueQuery(QuadtreeNodeIndex nodeIndex);
    void CHC_renderQuadtree(QuadtreeNodeIndex nodeIndex);

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
    unsigned int currentFrame;

    enum OcclusionQueriesAlgorithm
    {
        NONE,
        STOP_AND_WAIT,
        ADVANCED,
        CHC
    };

    using QueryInfo = std::pair<Query,glm::ivec2>;

    // Occlusion culling data (advanced)
    QueryPool queryPool;
    std::queue<QueryInfo> previousFrameQueries;
    std::unordered_set<glm::ivec2> PVS;

    // Occlusion culling data (CHC)
    Quadtree sceneHierarchy;
    int maxDepth;
    std::vector<std::vector<bool>> alreadyRendered;

};

#endif // _SCENE_INCLUDE
