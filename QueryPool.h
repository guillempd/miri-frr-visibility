#ifndef _QUERY_POOL_INCLUDE
#define _QUERY_POOL_INCLUDE

#include "Query.h"

#include "glm/glm.hpp"

#include "GL/glew.h"
#include "GL/gl.h"

#include <vector>

class QueryPool
{
public:
    QueryPool(int n);
    ~QueryPool();
    Query getQuery(const glm::ivec2 &gridCoordinates);
    void clear();
private:
    std::vector<GLuint> ids;
    int i;
};


#endif // _QUERY_POOL_INCLUDE