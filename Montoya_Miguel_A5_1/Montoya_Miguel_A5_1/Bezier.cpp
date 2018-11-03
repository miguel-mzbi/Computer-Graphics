#include <iostream>
#include <vector>
#include "GL/glui.h"
#include "Bezier.h"
#include "BuildGLUI.h"

using namespace std;

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|					Parameters						|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

// Basics 
int windowID;
int edge = 500;
extern GLUI *glui;

// Logic
float step = 1.0f; // Percentage of steps. Default 1%.
float deCastelT = 0.5f; // DeCasteljau algorithm t value to show. Default t = 0.5
int radioSelectedMode; // (0) Add control point, (1) Move control point, (2) Delete control point, (3) DeCasteljau algorithm
int drawCurve = 1;
int drawExtendedCurve = 0;
int drawControlPoly = 0;
int drawConvezHull = 0;

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|				Callback functions					|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

void displayFunction() {
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFlush();

	glFlush();
}

void resizeFunction(int width, int height) {
	if (width == 0 || height == 0) return;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	int x, y, w, h;
	GLUI_Master.get_viewport_area(&x, &y, &w, &h);

	int deltaH = 0, deltaW = 0, ed = 0;
	if (w > h) ed = h;
	else ed = w;

	deltaH = abs(h - ed) / 2;
	deltaW = abs(w - ed) / 2;
	glViewport(x + deltaW, y + deltaH, ed, ed);
}

void idleFunction() {
	if (glutGetWindow() != windowID) glutSetWindow(windowID);
}

void mouseFunction(int button, int state, int x, int y) {
	// If is not left button return
	if (button != GLUT_LEFT_BUTTON) return;
	if (state == GLUT_DOWN) {
		return;
	}
}

void motionFunction(int x, int y) {
	;
}


/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|					   Main							|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

int main(int argc, char** argv) {
	// Basic OpenGL initial setup
	glutInit(&argc, argv);
	glutInitWindowSize(edge, edge);
	glutInitWindowPosition(300, 300);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	windowID = glutCreateWindow("Montoya_Miguel A3");
	glutSetWindow(windowID);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Create GUI
	buildGLUI(windowID);

	// Start loop
	glutMainLoop();

	return 0;
}