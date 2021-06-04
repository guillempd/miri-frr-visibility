#include "QueryPool.h"

QueryPool::QueryPool(int n)
    : ids(n)
    , i(0)
{
    glGenQueries(ids.size(), ids.data());
}

QueryPool::QueryPool()
    : QueryPool(0)
    {}

QueryPool::~QueryPool()
{
    glDeleteQueries(ids.size(), ids.data());
}

// TODO: Check that i is in range (?)
Query QueryPool::getQuery()
{
    return Query(ids[i++]);
}

void QueryPool::clear()
{
    i = 0;
}