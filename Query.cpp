#include "Query.h"

Query::Query(int id, const glm::ivec2 &gridCoordinates)
    : id(id)
    , gridCoordinates(gridCoordinates)
    {}

// TODO: Check this works as expected
bool Query::isComplete() const
{
    GLint param;
    glGetQueryObjectiv(id, GL_QUERY_RESULT_AVAILABLE, &param);
    return param;
}

void Query::begin()
{
    glBeginQuery(GL_ANY_SAMPLES_PASSED, id);
}

void Query::end()
{
    glEndQuery(GL_ANY_SAMPLES_PASSED);
}

// TODO: Check this works as expected
bool Query::isVisible() const
{
    int param;
    glGetQueryObjectiv(id, GL_QUERY_RESULT, &param);
    return param;
}

glm::ivec2 Query::getPosition() const
{
    return gridCoordinates;
}