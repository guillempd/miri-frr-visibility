#include "Scene.h"
#include "Query.h"
#include "PLYReader.h"

#include "imgui.h"

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/hash.hpp>

#include <algorithm>
#include <iostream>

Scene::Scene()
{

}


Scene::~Scene()
{

}


void Scene::init()
{
    n = 16;
    frustumCulling = false;
    occlusionCulling = false;
    debugMode = false;
    pathMode = false;
    currentFrame = 0;

    initShaders();

    camera.init();
    loadMesh("../models/bunny.ply");
    cube.buildCube();
    cube.sendToOpenGL(basicProgram);
    floor.buildQuad();
    floor.sendToOpenGL(basicProgram);
    floorModel = glm::mat4(1.0f);
    floorModel = glm::translate(floorModel, glm::vec3(n/2 - 0.5, -0.5f, -n/2 + 0.5));
    floorModel = glm::rotate(floorModel, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
    floorModel = glm::scale(floorModel, glm::vec3(n));

    maxDepth = 4; // maxDepth = floor(log_2(n))
    buildSceneHierarchy();
}


void Scene::buildSceneHierarchy()
{
    // Number of nodes of a full quadtree with maxDepth
    int numNodes = (std::pow(4, maxDepth + 1) - 1)/ 3;
    sceneHierarchy.nodes = std::vector<QuadtreeNode>(numNodes);
    queryPool = QueryPool(numNodes);
    queryPool.clear();

    // Compute the bounding box of the root node
    QuadtreeNodeIndex rootIndex = sceneHierarchy.root();
    QuadtreeNode &root = sceneHierarchy.nodes[rootIndex];

    root.aabb.min = glm::vec3(0.0f, mesh.aabb.min.y, 0.0f);
    root.aabb.min += glm::vec3(-0.5f, 0.0f, 0.5f);
    root.aabb.max = glm::vec3(n, mesh.aabb.max.y, -n);
    root.aabb.max += glm::vec3(-0.5f, 0.0f, 0.5f);

    // Recursive function that builds the rest of the hierarchy
    buildSceneHierarchy(rootIndex);
}


void Scene::buildSceneHierarchy(QuadtreeNodeIndex nodeIndex)
{
    QuadtreeNode &node = sceneHierarchy.nodes[nodeIndex];
    node.visible = true;
    node.lastVisited = currentFrame;

    glm::vec2 aabbMin(node.aabb.min.x, node.aabb.min.z);
    glm::vec2 aabbMax(node.aabb.max.x, node.aabb.max.z);
    glm::vec2 aabbCenter = (aabbMin + aabbMax) / 2.0f;

    if (!sceneHierarchy.isLeaf(nodeIndex)) {
        float minY = node.aabb.min.y;
        float maxY = node.aabb.max.y;

        QuadtreeNodeIndex blChildIndex = 4 * nodeIndex + 1;
        QuadtreeNode &blChild = sceneHierarchy.nodes[blChildIndex];
        blChild.aabb.min = glm::vec3(aabbMin.x, minY, aabbMin.y);
        blChild.aabb.max = glm::vec3(aabbCenter.x, maxY, aabbCenter.y);

        QuadtreeNodeIndex brChildIndex = 4 * nodeIndex + 2;
        QuadtreeNode &brChild = sceneHierarchy.nodes[brChildIndex];
        brChild.aabb.min = glm::vec3(aabbCenter.x, minY, aabbMin.y);
        brChild.aabb.max = glm::vec3(aabbMax.x, maxY, aabbCenter.y);
        
        QuadtreeNodeIndex tlChildIndex = 4 * nodeIndex + 3;
        QuadtreeNode &tlChild = sceneHierarchy.nodes[tlChildIndex];
        tlChild.aabb.min = glm::vec3(aabbMin.x, minY, aabbCenter.y);
        tlChild.aabb.max = glm::vec3(aabbCenter.x, maxY, aabbMax.y);
        
        QuadtreeNodeIndex trChildIndex = 4 * nodeIndex + 4;
        QuadtreeNode &trChild = sceneHierarchy.nodes[trChildIndex];
        trChild.aabb.min = glm::vec3(aabbCenter.x, minY, aabbCenter.y);
        trChild.aabb.max = glm::vec3(aabbMax.x, maxY, aabbMax.y);

        buildSceneHierarchy(blChildIndex);
        buildSceneHierarchy(brChildIndex);
        buildSceneHierarchy(tlChildIndex);
        buildSceneHierarchy(trChildIndex);
    }
    else {
        node.gridPosition = glm::ivec2(aabbCenter.x, -aabbCenter.y);
    }
}


bool Scene::loadMesh(const char *filename)
{
    bool bSuccess = PLYReader::readMesh(filename, mesh);
    if (bSuccess) {
        mesh.sendToOpenGL(basicProgram);
        std::cout << "Mesh bounding box" << std::endl;
        std::cout << "min = (" << mesh.aabb.min.x << ", " << mesh.aabb.min.y << ", " << mesh.aabb.min.z << ")" << std::endl;
        std::cout << "max = (" << mesh.aabb.max.x << ", " << mesh.aabb.max.y << ", " << mesh.aabb.max.z << ")" << std::endl;
    }
    else std::cout << "Couldn't load mesh " << filename << std::endl;
    return bSuccess;
}


void Scene::update(int deltaTime)
{
    camera.update(deltaTime);
}


int Scene::render()
{
    if (ImGui::Begin("Settings")) {
        ImGui::Checkbox("Enable/Disable Frustum Culling", &frustumCulling);
        ImGui::Checkbox("Enable/Disable Path Recording Mode", &pathMode);
        ImGui::Checkbox("Enable/Disable Debug Mode", &debugMode);
        ImGui::Separator();
        ImGui::Text("Occlusion Culling Strategy");
        ImGui::RadioButton("None", &occlusionCulling, NONE);
        ImGui::RadioButton("Stop and Wait", &occlusionCulling, STOP_AND_WAIT);
        ImGui::RadioButton("Advanced", &occlusionCulling, ADVANCED);
        ImGui::RadioButton("CHC", &occlusionCulling, CHC);
    }
    ImGui::End();

    const glm::mat4 &view = camera.getViewMatrix();
    const glm::mat4 &projection = camera.getProjectionMatrix();
    basicProgram.use();
    basicProgram.setUniformMatrix4f("view", view);
    basicProgram.setUniformMatrix4f("projection", projection);
    basicProgram.setUniform1i("bLighting", 1);
    basicProgram.setUniform4f("color", 0.9f, 0.9f, 0.95f, 1.0f);

    ++currentFrame;
    renderFloor();
    switch(occlusionCulling) {
        case NONE:
            return renderBasic();
        case STOP_AND_WAIT:
            return renderStopAndWait();
        case ADVANCED:
            return renderAdvanced();
        case CHC:
            return renderCHC();
        default:
            std::cerr << "Unknown Occlusion Queries Algorithm" << std::endl;
            return -1;
    }
}


int Scene::renderBasic()
{
    int rendered = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            glm::ivec2 gridPosition(i, j);
            if (!frustumCulling || insideFrustum(gridPosition)) {
                render(gridPosition);
                ++rendered;
            }
        }
    }
    return rendered;
}


int Scene::renderStopAndWait()
{
    int rendered = 0;
    queryPool.clear();
    Query query = queryPool.getQuery();
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            glm::ivec2 gridPosition(i, j);
            if (frustumCulling && !insideFrustum(gridPosition)) continue;

            query.begin();
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glDepthMask(GL_FALSE);
            renderBoundingBox(gridPosition, false);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
            query.end();
            if (query.isVisible()) {
                render(gridPosition);
                ++rendered;
            }
        }
    }
    return rendered;
}


// Render in front to back order using visibility from previous frame (PVS)
// If object in PVS -> Render directly and issue query for next frame
// If object not in PVS -> Do not render and issue query to be resolved later this frame
// After all scene has been traverse, resolve the queries that were still pending for this frame
int Scene::renderAdvanced()
{
    // Front to back ordering of the scene
    std::vector<DistancePosition> E;
    E.reserve(n*n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            glm::ivec2 gridPosition(i, j);
            if (!frustumCulling || insideFrustum(gridPosition)) {
                float d = distanceToCamera(gridPosition);
                E.emplace_back(d, gridPosition);
            }
        }
    }

    auto compareFunction = [](const DistancePosition &x, const DistancePosition &y) {return x.first < y.first; };
    std::sort(E.begin(), E.end(), compareFunction);

    // Resolve visibility from previous frame
    while (!previousFrameQueries.empty()) {
        auto [query, gridPosition] = previousFrameQueries.front(); previousFrameQueries.pop();
        if (query.isVisible()) PVS.insert(gridPosition);
    }

    queryPool.clear();
    int rendered = 0;
    std::unordered_set<glm::ivec2> nextPVS;

    // Render front to back using visibility from previous frame
    std::queue<QueryInfo> currentFrameQueries;
    for (auto [d, gridPosition] : E) {

        // Check first if any of the queries of this frame is already available
        // If the result is available, and the object is visible, then render it first
        // This can help to reduce the number of objects drawn since this acts a blocker
        if (!currentFrameQueries.empty()) {
            auto [query, queryPosition] = currentFrameQueries.front();
            while (query.resultIsReady()) {
                currentFrameQueries.pop();
                if (query.isVisible()) {
                    render(queryPosition);
                    nextPVS.insert(queryPosition);
                    ++rendered;
                }
                if (currentFrameQueries.empty()) break;
                else {
                    auto [query_, queryPosition_] = currentFrameQueries.front();
                    query = query_;
                    queryPosition = queryPosition_;
                }
            }    
        }

        bool inV = (PVS.find(gridPosition) != PVS.end());
        if (inV) {
            Query query = queryPool.getQuery();
            query.begin();
            render(gridPosition);
            query.end();
            ++rendered;
            previousFrameQueries.emplace(query, gridPosition);
        }
        else { // !inV
            Query query = queryPool.getQuery();
            query.begin();
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glDepthMask(GL_FALSE);
            renderBoundingBox(gridPosition, false);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
            query.end();
            currentFrameQueries.emplace(query, gridPosition);
        }
    }

    // Resolve the visibility of this frame that is still unknown
    while (!currentFrameQueries.empty()) {
        auto [query, gridPosition] = currentFrameQueries.front(); currentFrameQueries.pop();
        if (query.isVisible()) {
            render(gridPosition);
            nextPVS.insert(gridPosition);
            ++rendered;
        }
    }

    PVS = std::move(nextPVS);
    return rendered;
}


// CHC implementation as 
int Scene::renderCHC()
{
    using QueryInfoCHC = std::pair<Query,QuadtreeNodeIndex>;
    alreadyRendered = std::vector<std::vector<bool>> (n, std::vector<bool>(n, false));

    std::stack<QuadtreeNodeIndex> nodes;
    std::queue<QueryInfoCHC> queries;
    int rendered = 0;
    queryPool.clear();

    nodes.push(sceneHierarchy.root());
    while (!nodes.empty() || !queries.empty()) {

        // If there are queries with result available, empty all of them
        if (!queries.empty()) {
            auto [query, nodeIndex] = queries.front();
            while (query.resultIsReady()) {
                queries.pop();

                if (query.isVisible()) {
                    pullUpVisibility(nodeIndex);

                    bool isLeaf = sceneHierarchy.isLeaf(nodeIndex);
                    if (isLeaf) rendered += render(nodeIndex);
                    else addChildren(nodeIndex, nodes);
                }

                if (queries.empty()) break;

                auto [query_, nodeIndex_] = queries.front();
                query = query_;
                nodeIndex = nodeIndex_;
            }
        }

        // Traverse the hierarchy of nodes using the previous frame visibility to render them
        if (!nodes.empty()) {
            QuadtreeNodeIndex nodeIndex = nodes.top(); nodes.pop();
            QuadtreeNode &node = sceneHierarchy.nodes[nodeIndex];

            bool wasVisible = node.visible && (node.lastVisited == currentFrame - 1);
            bool isLeaf = sceneHierarchy.isLeaf(nodeIndex);

            node.visible = false;
            node.lastVisited = currentFrame;
            
            // If node can be frustum culled there is nothing more to do
            if (!frustumCulling || insideFrustum(node.aabb)) {
                if (wasVisible) {
                    if (isLeaf) {
                        Query query = renderWithQuery(nodeIndex);
                        queries.emplace(query, nodeIndex);
                        ++rendered;
                    }
                    else addChildren(nodeIndex, nodes);
                }
                else {
                    Query query = issueQuery(nodeIndex);
                    queries.emplace(query, nodeIndex);
                }
            }
        }
    }
    return rendered;
}


// Add the children of the node sorted by distance to the camera (front to back rendering)
void Scene::addChildren(QuadtreeNodeIndex nodeIndex, std::stack<QuadtreeNodeIndex> &nodes)
{
    QuadtreeNodeIndex blChildIndex = 4 * nodeIndex + 1;
    QuadtreeNodeIndex brChildIndex = 4 * nodeIndex + 2;
    QuadtreeNodeIndex tlChildIndex = 4 * nodeIndex + 3;
    QuadtreeNodeIndex trChildIndex = 4 * nodeIndex + 4;

    QuadtreeNode &blChildNode = sceneHierarchy.nodes[blChildIndex];
    QuadtreeNode &brChildNode = sceneHierarchy.nodes[brChildIndex];
    QuadtreeNode &tlChildNode = sceneHierarchy.nodes[tlChildIndex];
    QuadtreeNode &trChildNode = sceneHierarchy.nodes[trChildIndex];

    std::vector<std::pair<float,QuadtreeNodeIndex>> v(4);
    
    v[0] = std::make_pair(distanceToCamera(blChildNode.gridPosition), blChildIndex);
    v[1] = std::make_pair(distanceToCamera(brChildNode.gridPosition), brChildIndex);
    v[2] = std::make_pair(distanceToCamera(tlChildNode.gridPosition), tlChildIndex);
    v[3] = std::make_pair(distanceToCamera(trChildNode.gridPosition), trChildIndex);

    std::sort(v.begin(), v.end());
    for (int i = 3; i >= 0; --i)
        nodes.push(v[i].second);
}


Query Scene::renderWithQuery(QuadtreeNodeIndex nodeIndex)
{
    QuadtreeNode &node = sceneHierarchy.nodes[nodeIndex];
    Query query = queryPool.getQuery();
    query.begin();
    render(node.gridPosition);
    query.end();
    alreadyRendered[node.gridPosition.x][node.gridPosition.y] = true;
    return query;
}


Query Scene::issueQuery(QuadtreeNodeIndex nodeIndex)
{
    bool isLeaf = sceneHierarchy.isLeaf(nodeIndex);
    Query query = queryPool.getQuery();
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    query.begin();
    if (isLeaf) {
        QuadtreeNode &node = sceneHierarchy.nodes[nodeIndex];
        renderBoundingBox(node.gridPosition, false);
    }
    else renderBoundingBox(nodeIndex, false);
    query.end();
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    return query;
}


// Set as visible the node and all of its ancestors
void Scene::pullUpVisibility(QuadtreeNodeIndex nodeIndex)
{
    QuadtreeNode &node = sceneHierarchy.nodes[nodeIndex];
    if (!node.visible) {
        node.visible = true;
        if (sceneHierarchy.hasParent(nodeIndex)) pullUpVisibility(sceneHierarchy.parent(nodeIndex));
    }
}


// Render the scene hierarchy, for debugging purposes
void Scene::renderSceneHierarchy(QuadtreeNodeIndex nodeIndex)
{
    renderBoundingBox(nodeIndex, true);
    if (!sceneHierarchy.isLeaf(nodeIndex)) {
        renderSceneHierarchy(4 * nodeIndex + 1);
        renderSceneHierarchy(4 * nodeIndex + 2);
        renderSceneHierarchy(4 * nodeIndex + 3);
        renderSceneHierarchy(4 * nodeIndex + 4);
    }
}


int Scene::render(QuadtreeNodeIndex nodeIndex)
{
    QuadtreeNode &node = sceneHierarchy.nodes[nodeIndex];
    if (alreadyRendered[node.gridPosition.x][node.gridPosition.y])
        return 0;
    else {
        render(node.gridPosition);
        alreadyRendered[node.gridPosition.x][node.gridPosition.y] = true;
        return 1;
    }
}


void Scene::render(const glm::ivec2 &gridPosition)
{
    const glm::mat4 model = glm::translate(glm::mat4(1.0f), worldPosition(gridPosition));
    const glm::mat4 &view = camera.getViewMatrix();
    const glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    basicProgram.setUniformMatrix4f("model", model);
    basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
    if (!pathMode) mesh.render();
    if (debugMode || pathMode) renderBoundingBox(gridPosition, true);
}


void Scene::renderBoundingBox(QuadtreeNodeIndex nodeIndex, bool wireframe)
{
    const QuadtreeNode &node = sceneHierarchy.nodes[nodeIndex];
    const AABB &aabb = node.aabb;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, (aabb.max + aabb.min) / 2.0f);
    model = glm::scale(model, aabb.max - aabb.min);

    renderBoundingBox(model, wireframe);
}


void Scene::renderBoundingBox(const glm::ivec2 &gridPosition, bool wireframe)
{
    const AABB &aabb = mesh.aabb;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, worldPosition(gridPosition));
    model = glm::scale(model, aabb.max - aabb.min);

    renderBoundingBox(model, wireframe);
}


void Scene::renderBoundingBox(const glm::mat4 &model, bool wireframe)
{
    const glm::mat4 &view = camera.getViewMatrix();
    const glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    basicProgram.setUniformMatrix4f("model", model);
    basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);

    if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    cube.render();
    if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void Scene::renderFloor()
{
    const glm::mat4 &view = camera.getViewMatrix();
    const glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(view * floorModel));
    basicProgram.setUniformMatrix4f("model", floorModel);
    basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
    floor.render();
}


bool Scene::insideFrustum(const glm::ivec2 &gridPosition) const
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), worldPosition(gridPosition));
    glm::vec3 aabbMin = model * glm::vec4(mesh.aabb.min, 1.0f);
    glm::vec3 aabbMax = model * glm::vec4(mesh.aabb.max, 1.0f);
    AABB aabb = {aabbMin, aabbMax};
    return insideFrustum(aabb);    
}


// Simple conservative frustum culling implementation, all computations are made in world space
// Checks for the existence of a frustum plane that leaves all vertices of the bounding box on the outside
// Might return false positives
bool Scene::insideFrustum(const AABB &aabb) const
{
    const Frustum &frustum = camera.getFrustum();
    glm::vec4 aabbMin = glm::vec4(aabb.min, 1.0f);
    glm::vec4 aabbIncrement = glm::vec4(aabb.max - aabb.min, 0.0f);

    for (const glm::vec4 &frustumPlane : frustum.planes) {
        bool allOutside = true;
        for (int x = 0; x <= 1 && allOutside; ++x) {
            for (int y = 0; y <= 1 && allOutside; ++y) {
                for (int z = 0; z <= 1 && allOutside; ++z) {
                    glm::vec4 aabbCorner = aabbMin + glm::vec4(x, y, z, 1.0f) * aabbIncrement;
                    if (glm::dot(frustumPlane, aabbCorner) <= 0.0f) allOutside = false;
                }
            }
        }
        if (allOutside) return false;
    }
    return true;
}


glm::vec3 Scene::worldPosition(const glm::ivec2 &gridPosition)
{
    return glm::vec3(gridPosition.x, 0, -gridPosition.y);
}


float Scene::distanceToCamera(const glm::ivec2 &gridPosition)
{
    return glm::distance(camera.getPosition(), worldPosition(gridPosition));
}


void Scene::initShaders()
{
    Shader vShader, fShader;

    vShader.initFromFile(VERTEX_SHADER, "shaders/basic.vs");
    if (!vShader.isCompiled())
    {
        std::cout << "Vertex Shader Error" << std::endl;
        std::cout << "" << vShader.log() << std::endl << std::endl;
    }
    fShader.initFromFile(FRAGMENT_SHADER, "shaders/basic.fs");
    if (!fShader.isCompiled())
    {
        std::cout << "Fragment Shader Error" << std::endl;
        std::cout << "" << fShader.log() << std::endl << std::endl;
    }
    basicProgram.init();
    basicProgram.addShader(vShader);
    basicProgram.addShader(fShader);
    basicProgram.link();
    if (!basicProgram.isLinked())
    {
        std::cout << "Shader Linking Error" << std::endl;
        std::cout << "" << basicProgram.log() << std::endl << std::endl;
    }
    basicProgram.bindFragmentOutput("fragColor");
    vShader.free();
    fShader.free();
}
