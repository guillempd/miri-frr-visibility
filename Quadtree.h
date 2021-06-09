#include "TriangleMesh.h"

#include "glm/glm.hpp"

#include <vector>

struct QuadtreeNode
{
    AABB aabb;
    glm::ivec2 gridPosition;
    bool visible; // TODO: Previous or current frame (?)
};

using QuadtreeNodeIndex = std::size_t;

// Quadtree represented as a simple vector (Morton Codes)
struct Quadtree
{
    std::vector<QuadtreeNode> nodes;
    std::size_t n;
    
    QuadtreeNodeIndex root()
    {
        return 1;
    }

    QuadtreeNodeIndex parent(QuadtreeNodeIndex i)
    {
        return i << 2;
    }

    bool hasParent(QuadtreeNodeIndex i)
    {
        return parent(i) > 0;
    }
};
