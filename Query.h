#ifndef _QUERY_INCLUDE
#define _QUERY_INCLUDE

#include <GL/glew.h>
#include <GL/gl.h>

class Query
{
public:
    Query(GLuint id);
    void begin() const;
    void end() const;
    bool isVisible() const;
private:
    GLuint id;
};

#endif