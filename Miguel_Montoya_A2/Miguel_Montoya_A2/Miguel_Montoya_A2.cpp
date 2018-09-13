// Miguel_Montoya_A2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <math.h>
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
static bool drawAxis = false;
static float prevAngle = 0;
static float currentAngle = 0;
static float deltaAngle = 0;
static vector<float> initialPoint(2); // Stores point P
static vector<float> currentPoint(2); // Stores point P' when moving mouse
static vector<float> currentQuaternion{ 0.0, 1.0, 1.0, 1.0 }; // Q(q, i, j, k) Stores quaternion applied to image shown on window
static vector<float> axisRotation{ 0.0, 0.0, 0.0 }; // Stores axis of rotation
static bool leftMouseActive = false;

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
	drawAxis = false;
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
	glLoadIdentity();
	fill(currentQuaternion.begin(), currentQuaternion.end(), 1);
	draw();
}

// Display function for teapot drawing
static void displayTea() {
    glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, C_WIDTH, C_HEIGHT);

	if (drawAxis) {
		glBegin(GL_LINES);
		// Red x axis
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(1.0f, 0.0f, 0.0f);
		// Green y axis
		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 1.0f, 0.0f);
		// Blue z axis
		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 1.0f);
		glEnd();
	}
	//glLoadIdentity();
	//glRotatef(currentQuaternion[0], currentQuaternion[1], currentQuaternion[2], currentQuaternion[3]);

	if (leftMouseActive) {
		glBegin(GL_LINES);
		glColor3f(1, 1, 0);
		glVertex3f(axisRotation[0], axisRotation[1], axisRotation[2]);
		glVertex3f(0, 0, 0);
		glEnd();
		glRotatef(deltaAngle, axisRotation[0], axisRotation[1], axisRotation[2]);
		deltaAngle = 0;
	}


	glColor3f(1.0, 1.0, 1.0);
	glutSolidTeapot(teaSize);
	draw();
}

static void screenToPlaneCoords(int x, int y, vector<float> &result) {
	result[0] = (x / ((float)C_WIDTH / 2) - 1.0);
	result[1] = -1.0f * (y / ((float)C_HEIGHT / 2) - 1.0);
}

static float computeAngle() {
	float y0 = initialPoint[1];
	float y1 = currentPoint[1];
	float deltaY = abs(y0 - y1);
	if (y1 > y0) {
		return deltaY * 360 / 2;
	}
	else {
		return -(deltaY * 360 / 2);
	}
}

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|				Callback functions					|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

// Mouse pressed callback
static void mouseCB(int button, int state, int x, int y) {
	// Skip non relevant clicks
	if (button == GLUT_RIGHT_BUTTON) {
		return;
	}
	else {
		if (state == GLUT_DOWN) {

			screenToPlaneCoords(x, y, initialPoint); // Returns coordinates for vector CP
			// Get vector PC (Vector PC = -CP)
			float vx = -initialPoint[0];
			float vy = -initialPoint[1];
			float vz = -1;
			vector<float> vectorPC{ vx, vy, vz }; // Vector PC declaration
			vector<float> vectorA{ vz, vy, -vx }; // 90° rotation of vector. This will be the axis of rotation A.
			float euclDist = sqrtf(pow(vx, 2) + pow(vy, 2) + pow(vz, 2)); // Eucledian distance for unit vector of the axis rotation
			// Unit vector A
			axisRotation[0] = vectorA[0] / euclDist;
			axisRotation[1] = vectorA[2] / euclDist;
			axisRotation[2] = vectorA[1] / euclDist;

			leftMouseActive = true;
		}
		else if (state == GLUT_UP) {
			
			vector<float> r = currentQuaternion;
			vector<float> q{ currentAngle, axisRotation[0], axisRotation[1], axisRotation[2] };
			float t0 = r[0] * q[0] - r[1] * q[1] - r[2] * q[2] - r[3] * q[3];
			float t1 = r[0] * q[1] + r[1] * q[0] - r[2] * q[3] + r[3] * q[2];
			float t2 = r[0] * q[2] + r[1] * q[3] + r[2] * q[0] - r[3] * q[1];
			float t3 = r[0] * q[3] - r[1] * q[2] + r[2] * q[1] + r[3] * q[0];
			vector<float> t{ t0,t1,t2,t3 };
			currentQuaternion = t;
			leftMouseActive = false;
			prevAngle = currentAngle = deltaAngle = 0;
		}
	}
}

static void mouseMovCB(int x, int y) {
	if (leftMouseActive) {
		prevAngle = currentAngle;
		screenToPlaneCoords(x, y, currentPoint);
		currentAngle = computeAngle();
		deltaAngle = currentAngle - prevAngle;
	}
}

static void reshapeCB(int width, int height) {
	if (width == 0 || height == 0) return;
	//Angle of view:40 degrees
	//Near clipping plane distance: 0.5
	//Far clipping plane distance: 20.0

	//gluPerspective(40.0, (GLdouble)width / (GLdouble)height, 0.5, 20.0);

	if (width > height) {
		C_HEIGHT = height;
		C_WIDTH = height;

	}
	else {
		C_HEIGHT = width;
		C_WIDTH = width;
	}
	displayTea();

}

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
	if (choice == 0) {
		drawAxis = !drawAxis;
	}
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
	glutReshapeFunc(reshapeCB);
	glutIdleFunc(displayTea);
	glutMouseFunc(mouseCB);
	glutMotionFunc(mouseMovCB);

	// displayTea();

	// Glut loop start
	glutMainLoop();
	return 0;
}