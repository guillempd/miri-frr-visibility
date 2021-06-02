#ifndef _APPLICATION_INCLUDE
#define _APPLICATION_INCLUDE

#include "Scene.h"

// Application is a singleton (a class with a single instance) that represents our whole app

class Application
{

public:
    Application() {}

    static Application &instance()
    {
        static Application G;

        return G;
    }

    void init();
    bool loadMesh(const char *filename);
    bool update(int deltaTime);
    void render();

    void resize(int width, int height);

    // Input callback methods
    void keyPressed(int key);
    void keyReleased(int key);
    void specialKeyPressed(int key);
    void specialKeyReleased(int key);
    void mouseMove(int x, int y);
    void mousePress(int button);
    void mouseRelease(int button);

    bool getKey(int key) const;
    bool getSpecialKey(int key) const;

    int getWidth() const;
    int getHeight() const;

private:

    void updateFrameRate(int deltaTime);
    bool bPlay;						  // Continue?
    Scene scene;					  // Scene to render
    bool keys[256], specialKeys[256]; // Store key states so that
                                      // we can have access at any time

    glm::ivec2 lastMousePos; // Last Mouse position
    bool mouseButtons[5];	 // State of mouse buttons

    bool bPolygonFill; // Draw filled faces or wireframe

    float mouseSensitivity;

    const int SAMPLE_TIME = 1000;
    int time;
    int frames;
    float fps;

    int width, height;
};

#endif // _APPLICATION_INCLUDE
