#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "Scene.h"
#include "PLYReader.h"

Scene::Scene()
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
    initShaders();
    mesh = new TriangleMesh();
    mesh->buildCube();
    mesh->sendToOpenGL(basicProgram);
    currentTime = 0.0f;

    camera.init();

    cube.buildCube();
    cube.sendToOpenGL(basicProgram);

    bPolygonFill = true;
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

int Scene::render(int n)
{
    int rendered = 0;
    const glm::mat4 &view = camera.getViewMatrix();
    const glm::mat4 &projection = camera.getProjectionMatrix();
    basicProgram.use();
    basicProgram.setUniformMatrix4f("view", view);
    basicProgram.setUniformMatrix4f("projection", projection);
    basicProgram.setUniform1i("bLighting", bPolygonFill ? 1 : 0);
    if (bPolygonFill)
    {
        basicProgram.setUniform4f("color", 0.9f, 0.9f, 0.95f, 1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else
    {
        basicProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(0.5f, 1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (render(i, j)) ++rendered;
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_POLYGON_OFFSET_FILL);
        basicProgram.setUniform4f("color", 0.0f, 0.0f, 0.0f, 1.0f);
    }

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (render(i, j)) ++rendered;

    return rendered;           
}

bool Scene::render(int i, int j)
{
    const glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(2*i, 0, -2*j));
    if (!insideFrustum(model)) return false;

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
