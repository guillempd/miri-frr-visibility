#include "Query.h"

Query::Query()
{
    glGenQueries(1, &id);
}

Query::~Query()
{
    glDeleteQueries(1, &id);
}

void Query::begin() const
{
    glBeginQuery(GL_ANY_SAMPLES_PASSED, id);
}

void Query::end() const
{
    glEndQuery(GL_ANY_SAMPLES_PASSED);
}

bool Query::isVisible() const
{
    GLint param;
    glGetQueryObjectiv(id, GL_QUERY_RESULT, &param);
    return param == GL_TRUE;
}