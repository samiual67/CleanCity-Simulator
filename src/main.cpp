// CleanCity Simulator - Smart City Waste Management & Cleanliness Monitoring Simulation
// C++ / OpenGL / FreeGLUT - entry point (GLUT callbacks only; everything else is in Application)
#include "GLInclude.h"
#include <cstdlib>
#include "Application.h"

static Application app;
static int lastTime = 0;

static void display() {
    app.render();
    glutSwapBuffers();
}

static void reshape(int w, int h) {
    app.setWindowSize(w, h);
}

static void timer(int) {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.05f) dt = 0.05f;                    // avoid huge jumps (window drag, breakpoints)
    app.update(dt);
    if (app.wantsExit) std::exit(0);
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);                   // ~60 FPS
}

static void mouse(int b, int s, int x, int y)   { app.onMouse(b, s, x, y); }
static void motion(int x, int y)                { app.onMotion(x, y); }
static void passive(int x, int y)               { app.onPassiveMotion(x, y); }
static void keyboard(unsigned char k, int, int) { app.onKey(k); }
static void special(int k, int, int)            { app.onSpecial(k); }
static void wheel(int, int dir, int x, int y)   { app.onWheel(dir, x, y); }

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(60, 40);
    glutCreateWindow("CleanCity Simulator - Smart City Waste Management Simulation");

    app.init();
    app.setWindowSize(WIN_W, WIN_H);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(passive);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMouseWheelFunc(wheel);
    lastTime = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(16, timer, 0);
    glutMainLoop();
    return 0;
}