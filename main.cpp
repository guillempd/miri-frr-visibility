#include <GL/glew.h>
#include <GL/freeglut.h>

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl3.h"

#include "Application.h"

#include <iostream>

//Remove console (only works in Visual Studio)
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

#define GLUT_SCROLL_UP      0x0003
#define GLUT_SCROLL_DOWN    0x0004

static int prevTime;
static bool capturingMouse;

void beginCapturingMouse() {
    capturingMouse = true;
    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);
    glutWarpPointer(width/2, height/2);
    glutSetCursor(GLUT_CURSOR_NONE);
}

void endCapturingMouse() {
    capturingMouse = false;
    glutSetCursor(GLUT_CURSOR_INHERIT);
}

// If a key is pressed this callback is called
static void keyboardDownCallback(unsigned char key, int x, int y)
{
    ImGui_ImplGLUT_KeyboardFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    if (key == 'i') {
        if (capturingMouse) endCapturingMouse();
        else beginCapturingMouse();
    }
    Application::instance().keyPressed(key);
}

// If a key is released this callback is called
static void keyboardUpCallback(unsigned char key, int x, int y)
{
    ImGui_ImplGLUT_KeyboardUpFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    Application::instance().keyReleased(key);
}

// If a special key is pressed this callback is called
static void specialDownCallback(int key, int x, int y)
{
    ImGui_ImplGLUT_SpecialFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    Application::instance().specialKeyPressed(key);
}

// If a special key is released this callback is called
static void specialUpCallback(int key, int x, int y)
{
    ImGui_ImplGLUT_SpecialFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    Application::instance().specialKeyReleased(key);
}

// Same for changes in mouse cursor position
static void motionCallback(int x, int y)
{
    if (capturingMouse) return;

    ImGui_ImplGLUT_MotionFunc(x, y);
}

static void passiveMotionCallback(int x, int y)
{
    if (!capturingMouse) return;

    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);
    Application::instance().mouseMove(x - width/2, y - height/2);
    if (x != width/2 || y != height/2) glutWarpPointer(width/2, height/2);
}

// Same for mouse button presses or releases
static void mouseCallback(int button, int state, int x, int y)
{
    if (button == GLUT_SCROLL_UP) ImGui_ImplGLUT_MouseWheelFunc(button, 1, x, y);
    else if (button == GLUT_SCROLL_DOWN) ImGui_ImplGLUT_MouseWheelFunc(button, -1, x, y);
    else ImGui_ImplGLUT_MouseFunc(button, state, x, y);
    if (ImGui::GetIO().WantCaptureMouse) return;

    int buttonId;

    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        buttonId = 0;
        break;
    case GLUT_RIGHT_BUTTON:
        buttonId = 1;
        break;
    case GLUT_MIDDLE_BUTTON:
        buttonId = 2;
        break;
    case GLUT_SCROLL_UP:
        buttonId = 3;
        break;
    case GLUT_SCROLL_DOWN:
        buttonId = 4;
        break;
    }

    if (state == GLUT_DOWN)
        Application::instance().mousePress(buttonId);
    else if (state == GLUT_UP)
        Application::instance().mouseRelease(buttonId);
}

// Resizing the window calls this function
static void resizeCallback(int width, int height)
{
    ImGui_ImplGLUT_ReshapeFunc(width, height);
    Application::instance().resize(width, height);
}

static void drawCallback()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();

    // Render the application frame (including calls to Dear ImGui)
    Application::instance().render();

    // Render the Dear ImGui frame (with the calls to Dear ImGui that the applcation has made)
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glutSwapBuffers();
}

static void idleCallback()
{
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    int deltaTime = currentTime - prevTime;

    // Every time we enter here is equivalent to a game loop execution
    if (!Application::instance().update(deltaTime)) glutLeaveMainLoop();
    prevTime = currentTime;
    glutPostRedisplay();
}

int main(int argc, char **argv)
{
    // GLUT initialization
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(640, 480);

    glutCreateWindow(argv[0]);
    glutReshapeFunc(resizeCallback);
    glutDisplayFunc(drawCallback);
    glutIdleFunc(idleCallback);
    glutKeyboardFunc(keyboardDownCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutSpecialFunc(specialDownCallback);
    glutSpecialUpFunc(specialUpCallback);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(motionCallback);
    glutPassiveMotionFunc(passiveMotionCallback);
    beginCapturingMouse();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Dear ImGui style
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGLUT_Init();
    ImGui_ImplOpenGL3_Init();

    // GLEW will take care of OpenGL extension functions
    glewExperimental = GL_TRUE;
    glewInit();

    // Application instance initialization
    Application::instance().init();
    prevTime = glutGet(GLUT_ELAPSED_TIME);
    // GLUT gains control of the application
    glutMainLoop();

    // Dear ImGui cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
