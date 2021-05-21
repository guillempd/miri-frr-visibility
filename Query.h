#ifndef _QUERY_INCLUDE
#define _QUERY_INCLUDE

#include <GL/glew.h>
#include <GL/gl.h>

class Query
{
public:
    Query(int id);
    bool isComplete() const;
    void begin();
    void end();
    bool isVisible() const;
private:
    GLuint id;
};

#endif // _QUERY_INCLUDE