#include "PLYReader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

bool PLYReader::readMesh(const std::string &filename, TriangleMesh &mesh)
{
    std::ifstream fin;
    int nVertices, nFaces;

    fin.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
    if (!fin.is_open())
        return false;
    if (!loadHeader(fin, nVertices, nFaces))
    {
        fin.close();
        return false;
    }

    std::vector<float> plyVertices;
    std::vector<int> plyTriangles;

    loadVertices(fin, nVertices, plyVertices);
    loadFaces(fin, nFaces, plyTriangles);
    fin.close();

    rescaleModel(plyVertices);
    addModelToMesh(plyVertices, plyTriangles, mesh);

    return true;
}

bool PLYReader::loadHeader(std::ifstream &fin, int &nVertices, int &nFaces)
{
    char line[100];

    fin.getline(line, 100);
    if (strncmp(line, "ply", 3) != 0)
        return false;
    nVertices = 0;
    fin.getline(line, 100);
    while (strncmp(line, "end_header", 10) != 0)
    {
        if (strncmp(line, "element vertex", 14) == 0)
            nVertices = atoi(&line[15]);
        fin.getline(line, 100);
        if (strncmp(line, "element face", 12) == 0)
            nFaces = atoi(&line[13]);
    }
    if (nVertices <= 0)
        return false;
    std::cout << "Loading triangle mesh" << std::endl;
    std::cout << "\tVertices = " << nVertices << std::endl;
    std::cout << "\tFaces = " << nFaces << std::endl;
    std::cout << std::endl;

    return true;
}

void PLYReader::loadVertices(std::ifstream &fin, int nVertices, std::vector<float> &plyVertices)
{
    int i;
    float value;

    for (i = 0; i < nVertices; i++)
    {
        fin.read((char *)&value, sizeof(float));
        plyVertices.push_back(value);
        fin.read((char *)&value, sizeof(float));
        plyVertices.push_back(value);
        fin.read((char *)&value, sizeof(float));
        plyVertices.push_back(value);
    }
}

void PLYReader::loadFaces(std::ifstream &fin, int nFaces, std::vector<int> &plyTriangles)
{
    int i, tri[3];
    unsigned char nVrtxPerFace;

    for (i = 0; i < nFaces; i++)
    {
        fin.read((char *)&nVrtxPerFace, sizeof(unsigned char));
        fin.read((char *)&tri[0], sizeof(int));
        fin.read((char *)&tri[1], sizeof(int));
        fin.read((char *)&tri[2], sizeof(int));
        plyTriangles.push_back(tri[0]);
        plyTriangles.push_back(tri[1]);
        plyTriangles.push_back(tri[2]);
        for (; nVrtxPerFace > 3; nVrtxPerFace--)
        {
            tri[1] = tri[2];
            fin.read((char *)&tri[2], sizeof(int));
            plyTriangles.push_back(tri[0]);
            plyTriangles.push_back(tri[1]);
            plyTriangles.push_back(tri[2]);
        }
    }
}

void PLYReader::rescaleModel(std::vector<float> &plyVertices)
{
    unsigned int i;
    glm::vec3 size[2];
    size[0] = glm::vec3(1e10, 1e10, 1e10);
    size[1] = glm::vec3(-1e10, -1e10, -1e10);

    for (i = 0; i < plyVertices.size(); i += 3)
    {
        size[0][0] = std::min(size[0][0], plyVertices[i]);
        size[0][1] = std::min(size[0][1], plyVertices[i + 1]);
        size[0][2] = std::min(size[0][2], plyVertices[i + 2]);
        size[1][0] = std::max(size[1][0], plyVertices[i]);
        size[1][1] = std::max(size[1][1], plyVertices[i + 1]);
        size[1][2] = std::max(size[1][2], plyVertices[i + 2]);
    }

    glm::vec3 center = size[1] - size[0];

    float largestSize = std::max(size[1][0] - size[0][0], std::max(size[1][1] - size[0][1], size[1][2] - size[0][2]));

    for (i = 0; i < plyVertices.size(); i += 3)
    {
        plyVertices[i] = (plyVertices[i] - center[0]) / largestSize;
        plyVertices[i + 1] = (plyVertices[i + 1] - center[1]) / largestSize;
        plyVertices[i + 2] = (plyVertices[i + 2] - center[2]) / largestSize;
    }
}

void PLYReader::addModelToMesh(const std::vector<float> &plyVertices, const std::vector<int> &plyTriangles, TriangleMesh &mesh)
{
    unsigned int i;

    for (i = 0; i < plyVertices.size(); i += 3)
        mesh.addVertex(glm::vec3(plyVertices[i], plyVertices[i + 1], plyVertices[i + 2]));
    for (i = 0; i < plyTriangles.size(); i += 3)
        mesh.addTriangle(plyTriangles[i], plyTriangles[i + 1], plyTriangles[i + 2]);
}
