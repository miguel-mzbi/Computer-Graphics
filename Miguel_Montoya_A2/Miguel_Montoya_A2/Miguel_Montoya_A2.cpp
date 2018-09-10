// Miguel_Montoya_A2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "GL/freeglut.h"
#include "functions.h"

using namespace std;

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|					Parameters						|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

static int C_WIDTH = 700;
static int C_HEIGHT = 700;
static int windowID;
static float teaSize = 0.5;

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|				Utility functions					|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

static void draw() {
	glutSwapBuffers();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Clear both buffers
static void clearBothBuffers() {
	// Clean back buffer
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Swap
	glutSwapBuffers();
	// Clean new back buffer
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void refresh() {
	displayTea();
}

// Display function for teapot drawing
static void displayTea() {
    glMatrixMode(GL_MODELVIEW);
	glutSolidTeapot(teaSize);
	draw();
}

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|				Callback functions					|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

// Other action submenu callback
static void otherSubMenuCB(int choice) {
	// Clear
	if (choice == 0) {
		clearBothBuffers();
	}
	// Refresh
	else if (choice == 1) {
		refresh();
	}
	// Exit
	else if (choice == 2) {
		glutDestroyWindow(windowID);
		exit(0);
	}
}

// Rotate submenu callback
static void rotateSubMenuCB(int choice) {

}

// Diplay submenu callback
static void displaySubMenuCB(int choice) {

}

// Display callback
static void displayCB() {
}

void initMenus() {
	// Other action submenu
	int otherSM = glutCreateMenu(otherSubMenuCB);
	glutAddMenuEntry("Clear", 0);
	glutAddMenuEntry("Refresh", 1);
	glutAddMenuEntry("Exit", 2);
	// Rotate action submenu
	int rotateSM = glutCreateMenu(rotateSubMenuCB);
	glutAddMenuEntry("Rotate world", 0);
	// Display action submenu
	int displaySM = glutCreateMenu(displaySubMenuCB);
	glutAddMenuEntry("Show axes", 0);

	// Menu setup
	glutCreateMenu(NULL);
	glutAddSubMenu("Other Action", otherSM);
	glutAddSubMenu("Rotate", rotateSM);
	glutAddSubMenu("Display", displaySM);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|					   Main							|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

int main(int argc, char **argv) {
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(C_WIDTH, C_HEIGHT);
	windowID = glutCreateWindow("Montoya5 assignement 2");
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	clearBothBuffers();

	// Initialize menus
	initMenus();

	/*xRotated = yRotated = zRotated = 30.0;
	xRotated = 33;
	yRotated = 40;*/

	// Event function
	glutDisplayFunc(displayCB);
	//glutReshapeFunc(reshapeFunc);
	//glutIdleFunc(idleFunc);

	displayTea();

	// Glut loop start
	glutMainLoop();
	return 0;
}