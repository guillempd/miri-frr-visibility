#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "Scene.h"
#include "PLYReader.h"

#include <algorithm>
#include <utility>

Scene::Scene()
    : queryPool(16*16)
{
    mesh = NULL;
}

Scene::~Scene()
{
    if (mesh != NULL)
        delete mesh;
    cube.free();
}

void Scene::init()
{
    n = 16;

    initShaders();
    
    mesh = new TriangleMesh();
    mesh->buildCube();
    mesh->sendToOpenGL(basicProgram);
    currentTime = 0.0f;

    camera.init();

    cube.buildCube();
    cube.sendToOpenGL(basicProgram);

    bPolygonFill = true;

    entities = std::vector<std::vector<Entity>>(n, std::vector<Entity>(n));
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            entities[i][j].wasVisible = true;
        }
    }
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
    camera.update(deltaTime);
}

int Scene::render(int n, bool frustumCulling, bool occlusionCulling)
{

    const glm::mat4 &view = camera.getViewMatrix();
    const glm::mat4 &projection = camera.getProjectionMatrix();
    basicProgram.use();
    basicProgram.setUniformMatrix4f("view", view);
    basicProgram.setUniformMatrix4f("projection", projection);
    basicProgram.setUniform1i("bLighting", bPolygonFill ? 1 : 0);
    basicProgram.setUniform4f("color", 0.9f, 0.9f, 0.95f, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (occlusionCulling) return renderWithOcclusionCulling();

    int rendered = 0;
    if (!bPolygonFill)
    {
        basicProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(0.5f, 1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (render(i, j, frustumCulling, occlusionCulling)) ++rendered;
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_POLYGON_OFFSET_FILL);
        basicProgram.setUniform4f("color", 0.0f, 0.0f, 0.0f, 1.0f);
    }


    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (render(i, j, frustumCulling, occlusionCulling)) ++rendered;

    return rendered;           
}

bool Scene::render(int i, int j, bool frustumCulling, bool occlusionCulling)
{
    const glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(i, 0, -j));

    if (frustumCulling && !insideFrustum(model)) return false;

    const glm::mat4 &view = camera.getViewMatrix();
    const glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    basicProgram.setUniformMatrix4f("model", model);
    basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
    
    mesh->render();
    return true;
}

// Simple conservative frustum culling implementation, all computations are made in world space
// Checks for the existence of a frustum plane that leaves all vertices of the bounding box to the outside side
// Might return false positives
// For more information see: https://www.iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
bool Scene::insideFrustum(const glm::mat4 &model) const
{
    const Frustum &frustum = camera.getFrustum();
    glm::vec4 aabbMin = model * glm::vec4(mesh->aabb.min, 1.0f);
    glm::vec4 aabbIncrement = model * glm::vec4(mesh->aabb.max - mesh->aabb.min, 0.0f);

    for (int i = 0; i < frustum.planes.size(); ++i) {
        glm::vec4 frustumPlane = frustum.planes[i];
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

// TODO: This should be private member of scene
static glm::vec3 objectPosition(glm::ivec2 position)
{
    return glm::vec3(position.x, 0, -position.y);
}


void Scene::render(glm::ivec2 position)
{
    const glm::mat4 model = glm::translate(glm::mat4(1.0f), objectPosition(position));
    const glm::mat4 &view = camera.getViewMatrix();
    const glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    basicProgram.setUniformMatrix4f("model", model);
    basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
    
    mesh->render();
}

// TODO: This should render the bounding cube
void Scene::renderQueryBox(glm::ivec2 position)
{
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);

    const glm::mat4 model = glm::translate(glm::mat4(1.0f), objectPosition(position));
    const glm::mat4 &view = camera.getViewMatrix();
    const glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    basicProgram.setUniformMatrix4f("model", model);
    basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
    
    mesh->render();

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
}


struct ComparisonFunction
{
    bool operator()(const std::pair<float, glm::ivec2> &lhs, const std::pair<float, glm::ivec2> &rhs)
    {
        return lhs.first < rhs.first;
    }
};

int Scene::renderWithOcclusionCulling()
{
    const int n = 16;
    int rendered = 0;
    glm::vec3 cameraPosition = camera.getPosition();

    // Resolve previous frame queries
    while (!previousFrameQueries.empty()) {
        const Query &query = previousFrameQueries.front();
        previousFrameQueries.pop();
        glm::ivec2 position = query.getPosition();
        entities[position.x][position.y].wasVisible = query.isVisible();
    }
    queryPool.clear();

    // Render front to back
    std::vector<std::pair<float, glm::ivec2>> entitiesPosition;
    entities.reserve(n * n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            glm::vec3 displacement = objectPosition(glm::ivec2(i, j)) - cameraPosition;
            float distanceSq = glm::dot(displacement, displacement);
            entitiesPosition.push_back(std::make_pair(distanceSq, glm::ivec2(i, j)));
        }
    }
    ComparisonFunction func;
    std::sort(entitiesPosition.begin(), entitiesPosition.end(), func);

    std::queue<Query> currentFrameQueries;
    for (auto entityPosition : entitiesPosition) {
        glm::ivec2 position = entityPosition.second;
        Entity entity = entities[position.x][position.y];
        if (entity.wasVisible) {
            Query query = queryPool.getQuery(position);
            query.begin();
            render(position);
            ++rendered;
            query.end();
            previousFrameQueries.push(query);
        }
        else {
            Query query = queryPool.getQuery(position);
            query.begin();
            renderQueryBox(position);
            query.end();
            currentFrameQueries.push(query);
        }
    }

    // Resolve this frame queries and possibly render the few remaining
    while (!currentFrameQueries.empty()) {
        const Query &query = currentFrameQueries.front();
        currentFrameQueries.pop();
        if (query.isVisible()) {
            glm::ivec2 position = query.getPosition();
            entities[position.x][position.y].wasVisible = true;
            render(position);
            ++rendered;
        }
    }

    return rendered;
}

Camera &Scene::getCamera()
{
    return camera;
}

void Scene::switchPolygonMode()
{
    bPolygonFill = !bPolygonFill;
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
