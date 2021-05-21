#include "QueryPool.h"

QueryPool::QueryPool(int n)
    : ids(n)
{
    glGenQueries(n, ids.data());
}

QueryPool::~QueryPool()
{
    glDeleteQueries(ids.size(), ids.data());
}

Query QueryPool::getQuery(const glm::ivec2 &gridCoordinates)
{
    return Query(ids[i++], gridCoordinates);
}

void QueryPool::clear()
{
    i = 0;
}