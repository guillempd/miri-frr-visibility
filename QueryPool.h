#ifndef _INCLUDE_QUERY_POOL
#define _INCLUDE_QUERY_POOL

#include "Query.h"

#include <vector>

class QueryPool
{
public:
    QueryPool(int n);
    QueryPool();
    ~QueryPool();
    Query getQuery();
    void clear();
private:
    std::vector<GLuint> ids;
    int i;
};

#endif // _INCLUDE_QUERY_POOL