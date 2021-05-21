#ifndef _QUERY_INCLUDE
#define _QUERY_INCLUDE

#include "glm/glm.hpp"

#include <GL/glew.h>
#include <GL/gl.h>

class Query
{
public:
    Query(int id, const glm::ivec2 &gridCoordinates);
    bool isComplete() const;
    void begin();
    void end();
    bool isVisible() const;
    glm::ivec2 getPosition() const;
private:
    GLuint id;
    glm::ivec2 gridCoordinates;
};

#endif // _QUERY_INCLUDE