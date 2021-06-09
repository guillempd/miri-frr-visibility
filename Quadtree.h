#include "TriangleMesh.h"

#include "glm/glm.hpp"

#include <vector>

struct QuadtreeNode
{
    AABB aabb;
    glm::ivec2 gridPosition; // Used for leaf nodes to render the object
    bool visible; // TODO: Previous or current frame (?)
    unsigned int lastVisited;
};

using QuadtreeNodeIndex = std::size_t;

// Quadtree represented as a simple vector (Morton Codes)
struct Quadtree
{
    std::vector<QuadtreeNode> nodes;
    
    QuadtreeNodeIndex root()
    {
        return 0;
    }

    QuadtreeNodeIndex parent(QuadtreeNodeIndex i)
    {
        return (i-1)/4;
    }

    bool hasParent(QuadtreeNodeIndex i)
    {
        return i > 0;
    }

    bool isLeaf(QuadtreeNodeIndex i)
    {
        QuadtreeNodeIndex child = 4 * i + 1;
        return child >= nodes.size();
    }
};
