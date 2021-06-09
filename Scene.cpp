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
#include <unordered_set>

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

    queryPool = QueryPool(n*n);
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

using DistancePosition = std::pair<float,glm::ivec2>;

glm::vec3 Scene::worldPosition(const glm::ivec2 &gridPosition)
{
    return glm::vec3(gridPosition.x, 0, -gridPosition.y);
}

float Scene::distanceToCamera(const glm::ivec2 &gridPosition)
{
    return glm::distance(camera.getPosition(), worldPosition(gridPosition));
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

int Scene::renderCHC()
{
    return 0;
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

// TODO: Can make this more efficient by reducing uniform passing since they are already passed in some cases
void Scene::renderBoundingBox(const glm::ivec2 &gridPosition, bool wireframe)
{
    const AABB &aabb = mesh.aabb;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, worldPosition(gridPosition));
    model = glm::scale(model, aabb.max - aabb.min);
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

// Simple conservative frustum culling implementation, all computations are made in world space
// Checks for the existence of a frustum plane that leaves all vertices of the bounding box to the outside side
// Might return false positives
// For more information see: https://www.iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
bool Scene::insideFrustum(const glm::ivec2 &gridPosition) const
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), worldPosition(gridPosition));
    const Frustum &frustum = camera.getFrustum();
    glm::vec4 aabbMin = model * glm::vec4(mesh.aabb.min, 1.0f);
    glm::vec4 aabbIncrement = model * glm::vec4(mesh.aabb.max - mesh.aabb.min, 0.0f);

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
