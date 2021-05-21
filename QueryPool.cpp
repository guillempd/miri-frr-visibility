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

Query QueryPool::getQuery()
{
    return Query(ids[i++]);
}

void QueryPool::clear()
{
    i = 0;
}