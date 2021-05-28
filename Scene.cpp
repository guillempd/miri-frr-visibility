#include "Scene.h"
#include "FlywayCamera.h"
#include "PathCamera.h"
#include "PLYReader.h"

#include "imgui.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <algorithm>
#include <iostream>
#include <utility>

Scene::Scene()
    // : queryPool(16*16)
{
    mesh = NULL;
}

Scene::~Scene()
{
    if (mesh != NULL)
        delete mesh;
    cube.free();
    floor.free();
}

void Scene::init()
{
    n = 16;
    frustumCulling = false;
    occlusionCulling = false;
    debug = false;

    initShaders();
    
    mesh = new TriangleMesh();
    mesh->buildCube();
    mesh->sendToOpenGL(basicProgram);
    currentTime = 0.0f;

    camera = new FlywayCamera;
    camera->init();

    cube.buildCube();
    cube.sendToOpenGL(basicProgram);

    floor.buildQuad();
    floor.sendToOpenGL(basicProgram);

    floorModel = glm::mat4(1.0f);
    floorModel = glm::translate(floorModel, glm::vec3(n/2 - 0.5, -0.5f, -n/2 + 0.5));
    floorModel = glm::rotate(floorModel, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
    floorModel = glm::scale(floorModel, glm::vec3(n));

    // entities = std::vector<std::vector<Entity>>(n, std::vector<Entity>(n));
    // for (int i = 0; i < n; ++i) {
    //     for (int j = 0; j < n; ++j) {
    //         entities[i][j].wasVisible = true;
    //     }
    // }
}

bool Scene::loadMesh(const char *filename)
{
    PLYReader reader;

    mesh->free();
    bool bSuccess = reader.readMesh(filename, *mesh);
    if (bSuccess) {
        mesh->sendToOpenGL(basicProgram);
        std::cout << "Mesh bounding box" << std::endl;
        std::cout << "min = (" << mesh->aabb.min.x << ", " << mesh->aabb.min.y << ", " << mesh->aabb.min.z << ")" << std::endl;
        std::cout << "max = (" << mesh->aabb.max.x << ", " << mesh->aabb.max.y << ", " << mesh->aabb.max.z << ")" << std::endl;
    }
    else std::cout << "Couldn't load mesh " << filename << std::endl;
    return bSuccess;
}

void Scene::update(int deltaTime)
{
    currentTime += deltaTime;
    if (!camera->update(deltaTime))
    {
        // This signals that the path camera has finished its path
        endReplayCameraPath();
    }
}

int Scene::render()
{
    if (ImGui::Begin("Settings")) {
        ImGui::Checkbox("Enable/Disable Frustum Culling", &frustumCulling);
        ImGui::Checkbox("Enable/Disable Occlusion Culling", &occlusionCulling);
        ImGui::Checkbox("Enable/Disable Debug Mode", &debug);

        ImGui::InputText("input file", inputBuff, IM_ARRAYSIZE(inputBuff));
        ImGui::SameLine();
        if (ImGui::Button("Replay Camera Path")) {
            beginReplayCameraPath(inputBuff);
        }

        ImGui::InputText("output file", outputBuff, IM_ARRAYSIZE(outputBuff));
        ImGui::SameLine();
        if (ImGui::Button("Record Camera Path")) {
            recordCameraPath(outputBuff, 10);
        }
    }
    ImGui::End();

    const glm::mat4 &view = camera->getViewMatrix();
    const glm::mat4 &projection = camera->getProjectionMatrix();
    basicProgram.use();
    basicProgram.setUniformMatrix4f("view", view);
    basicProgram.setUniformMatrix4f("projection", projection);
    basicProgram.setUniform1i("bLighting", 1);
    basicProgram.setUniform4f("color", 0.9f, 0.9f, 0.95f, 1.0f);

    renderFloor();
    if (frustumCulling && occlusionCulling) return renderUltimate(); 
    else if (occlusionCulling) return renderOcclusionCulling();
    else if (frustumCulling) return renderFrustumCulling();
    else return renderBasic();
}

int Scene::renderBasic()
{
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            render(glm::ivec2(i, j));
        }
    }
    return n * n;
}

int Scene::renderFrustumCulling()
{
    int rendered = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            glm::ivec2 gridPosition(i, j);
            if (insideFrustum(gridPosition)) {
                render(gridPosition);
                ++rendered;
            }
        }
    }
    return rendered;
}

// Stop and wait occlusion culling method
int Scene::renderOcclusionCulling()
{
    int rendered = 0;
    GLuint id;
    glGenQueries(1, &id);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            glm::ivec2 gridPosition(i, j);

            glBeginQuery(GL_ANY_SAMPLES_PASSED, id);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glDepthMask(GL_FALSE);
            renderBoundingBox(gridPosition, false);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
            glEndQuery(GL_ANY_SAMPLES_PASSED);

            GLint visible;
            glGetQueryObjectiv(id, GL_QUERY_RESULT, &visible);
            if (visible == GL_TRUE) {
                render(gridPosition);
                ++rendered;
            }
        }
    }
    glDeleteQueries(1, &id);
    return rendered;
}

// TODO: Combine Frustum Culling + Occlusion Culling for ultimate performance
int Scene::renderUltimate()
{
    int rendered = 0;
    GLuint id;
    glGenQueries(1, &id);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            glm::ivec2 gridPosition(i, j);
            if (!insideFrustum(gridPosition)) continue;

            glBeginQuery(GL_ANY_SAMPLES_PASSED, id);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glDepthMask(GL_FALSE);
            renderBoundingBox(gridPosition, false);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
            glEndQuery(GL_ANY_SAMPLES_PASSED);

            GLint visible;
            glGetQueryObjectiv(id, GL_QUERY_RESULT, &visible);
            if (visible == GL_TRUE) {
                render(gridPosition);
                ++rendered;
            }
        }
    }
    glDeleteQueries(1, &id);
    return rendered;
}

glm::vec3 Scene::worldPosition(const glm::ivec2 &gridPosition)
{
    return glm::vec3(gridPosition.x, 0, -gridPosition.y);
}

void Scene::render(const glm::ivec2 &gridPosition)
{
    const glm::mat4 model = glm::translate(glm::mat4(1.0f), worldPosition(gridPosition));
    const glm::mat4 &view = camera->getViewMatrix();
    const glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    basicProgram.setUniformMatrix4f("model", model);
    basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
    
    mesh->render();
    if (debug) renderBoundingBox(gridPosition, true);
}

// TODO: Can make this more efficient by reducing uniform passing since they are already passed in some cases
void Scene::renderBoundingBox(const glm::ivec2 &gridPosition, bool wireframe)
{
    const AABB &aabb = mesh->aabb;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, worldPosition(gridPosition));
    model = glm::scale(model, aabb.max - aabb.min);
    const glm::mat4 &view = camera->getViewMatrix();
    const glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    basicProgram.setUniformMatrix4f("model", model);
    basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);

    if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    cube.render();
    if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Scene::renderFloor()
{
    const glm::mat4 &view = camera->getViewMatrix();
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
    const Frustum &frustum = camera->getFrustum();
    glm::vec4 aabbMin = model * glm::vec4(mesh->aabb.min, 1.0f);
    glm::vec4 aabbIncrement = model * glm::vec4(mesh->aabb.max - mesh->aabb.min, 0.0f);

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

// struct ComparisonFunction
// {
//     bool operator()(const std::pair<float, glm::ivec2> &lhs, const std::pair<float, glm::ivec2> &rhs)
//     {
//         return lhs.first < rhs.first;
//     }
// };

// int Scene::renderWithOcclusionCulling()
// {
//     const int n = 16;
//     int rendered = 0;
//     glm::vec3 cameraPosition = camera.getPosition();

//     // Resolve previous frame queries
//     while (!previousFrameQueries.empty()) {
//         const Query &query = previousFrameQueries.front();
//         previousFrameQueries.pop();
//         glm::ivec2 position = query.getPosition();
//         entities[position.x][position.y].wasVisible = query.isVisible();
//     }
//     queryPool.clear();

//     // Render front to back
//     std::vector<std::pair<float, glm::ivec2>> entitiesPosition;
//     entities.reserve(n * n);
//     for (int i = 0; i < n; ++i) {
//         for (int j = 0; j < n; ++j) {
//             glm::vec3 displacement = objectPosition(glm::ivec2(i, j)) - cameraPosition;
//             float distanceSq = glm::dot(displacement, displacement);
//             entitiesPosition.push_back(std::make_pair(distanceSq, glm::ivec2(i, j)));
//         }
//     }
//     ComparisonFunction func;
//     std::sort(entitiesPosition.begin(), entitiesPosition.end(), func);

//     std::queue<Query> currentFrameQueries;
//     for (auto entityPosition : entitiesPosition) {
//         glm::ivec2 position = entityPosition.second;
//         Entity entity = entities[position.x][position.y];
//         if (entity.wasVisible) {
//             Query query = queryPool.getQuery(position);
//             query.begin();
//             render(position);
//             ++rendered;
//             query.end();
//             previousFrameQueries.push(query);
//         }
//         else {
//             Query query = queryPool.getQuery(position);
//             query.begin();
//             renderQueryBox(position);
//             query.end();
//             currentFrameQueries.push(query);
//         }
//     }

//     // Resolve this frame queries and possibly render the few remaining
//     while (!currentFrameQueries.empty()) {
//         const Query &query = currentFrameQueries.front();
//         currentFrameQueries.pop();
//         if (query.isVisible()) {
//             glm::ivec2 position = query.getPosition();
//             entities[position.x][position.y].wasVisible = true;
//             render(position);
//             ++rendered;
//         }
//     }

//     return rendered;
// }

ICamera &Scene::getCamera()
{
    return *camera;
}

void Scene::beginReplayCameraPath(const std::string &path)
{
    delete camera;
    camera = new PathCamera(path);
    camera->init();
}

void Scene::endReplayCameraPath()
{
    delete camera;
    camera = new FlywayCamera;
    camera->init();
}

void Scene::recordCameraPath(const std::string &path, int duration)
{
    camera->recordPath(path, duration);
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
