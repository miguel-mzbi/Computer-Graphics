// Montoya_Miguel_A2.cpp : This file contains the 'main' function. Program execution begins and ends there.
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

static int windowID;

static int C_EDGE = 700;
static const float PI = atanf(1) * 4;

static float teaSize = 0.5; // Teapot size

static bool drawAxis = false; // Draw axis if requested
static float cleared = false; // If screen is cleared, nothing will be shown
static bool leftMouseActive = false; // If true, mouse movement is active and must draw axis and apply rotation

static float currentAngle = 0; // Stores angles of rotation according to mouse drag

static vector<float> initialPoint(2); // Stores point P
static vector<float> currentPoint(2); // Stores point P' when moving mouse
static vector<float> currentQuaternion{ 1, 0, 0, 0 }; // Q(q, i, j, k) Stores quaternion applied to image shown on window
static vector<float> axisRotation{ 0.0, 0.0, 0.0 }; // Stores axis of rotation

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|				Utility functions					|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

// Draws into buffer and swaps. Cleans previous buffer.
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

// Display teapot again, with all initial values.
static void refresh() {
	displayTea();
}

// Display function for teapot drawing
static void displayTea() {
	if (cleared) return;
	
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	// Draw axis if enabled
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
		glEnd();
	}

	// Rotate world according to current quaternion
	// Quaternion must be translates to axis-angle system using arc-cos(phi)*2, and then convert to degrees from radians
	glRotatef((180/PI)*(acos(currentQuaternion[0]))*2, currentQuaternion[1], currentQuaternion[2], currentQuaternion[3]);
	
	// If left mouse is active, draw axis of rotation and rotate
	if (leftMouseActive) {
		glBegin(GL_LINES);
		glColor3f(1, 1, 0);
		glVertex3f(axisRotation[0], axisRotation[1], axisRotation[2]);
		glVertex3f(0, 0, 0);
		glEnd();
		glRotatef(currentAngle, axisRotation[0], axisRotation[1], axisRotation[2]);
	}

	// Draw white teapot
	glColor3f(1.0, 1.0, 1.0);
	glutSolidTeapot(teaSize);
	draw();
}

// Translate pixel coordinates to point in z=0 plane
static void screenToPlaneCoords(int x, int y, vector<float> &result) {
	int currentWidth = glutGet(GLUT_WINDOW_WIDTH);
	int currentHeight = glutGet(GLUT_WINDOW_HEIGHT);
	// Adjust coordinates to border created by resizing
	int borderX = 0, borderY = 0;
	if (currentWidth <= currentHeight) {
		borderY = (currentHeight-currentWidth)/2;
		borderX = 0;
	}
	else {
		borderY = 0;
		borderX = (currentWidth - currentHeight) / 2;
	}
	result[0] = ((x - borderX) / ((float)C_EDGE / 2) - 1.0f);
	result[1] = -1.0f * ((y - borderY) / ((float)C_EDGE / 2) - 1.0f);
}

// Computes angle for temporal rotation, according to perpendicular distance of the mouse to the line.
static float computeAngle() {
	float yR = currentPoint[1];
	float xR = currentPoint[0];
	float yI = initialPoint[1];
	float xI = initialPoint[0];
	
	// Get rid of division by 0
	float a, b;
	if (axisRotation[0] == 0) {
		a = 0;
	}
	else {
		a = 1 / axisRotation[0];
	}
	if (axisRotation[1] == 0) {
		b = 0;
	}
	else {
		b = 1 / axisRotation[1];
	}

	float distToAxis = (xR*a - yR*b) / sqrt(a*a + b*b); // Current mouse distance to axis.
	float currentDistanceAxis = (xI*a - yI*b) / sqrt(a*a + b * b); // Distance from initial point to axis.
	
	if (currentDistanceAxis - distToAxis >= 0) {
		return abs(currentDistanceAxis - distToAxis) * 360 / 2;
	}
	else {
		return -(abs(currentDistanceAxis - distToAxis) * 360 / 2);
	}
}

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|				Callback functions					|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

// Mouse pressed CB
static void mouseCB(int button, int state, int x, int y) {
	// Skip non relevant clicks
	if (button == GLUT_RIGHT_BUTTON) {
		return;
	}
	else {
		if (state == GLUT_DOWN) {
			// Returns coordinates for vector CP
			screenToPlaneCoords(x, y, initialPoint); 
			// Creates axis of rotation
			axisRotation[0] = initialPoint[1];
			axisRotation[1] = -initialPoint[0];
			axisRotation[2] = 0;

			leftMouseActive = true;
		}
		// On mouse up, add last rotation permanently to current quaterion.
		else if (state == GLUT_UP) {
			// Calculate K scalar for vector axis (normalize). z component of axis is not considered, as its always 0.
			float k = sin((currentAngle*PI) / (180 * 2.0f)) / sqrt(axisRotation[0] * axisRotation[0] + axisRotation[1] * axisRotation[1]);
			// Copy current quaternion
			vector<float> c{ currentQuaternion[0], currentQuaternion[1], currentQuaternion[2], currentQuaternion[3] };
			// Convert angle-axis system to quaternion, applying k to axis components, and storing the angle in radians and as cos(phi/2)
			vector<float> d{ cos((currentAngle*PI)/(180*2.0f)), k*axisRotation[0], k*axisRotation[1], axisRotation[2] };
			// Quaternion multiplication
			float t0 = d[0]*c[0] - d[1]*c[1] - d[2]*c[2] - d[3]*c[3];
			float t1 = d[0]*c[1] + d[1]*c[0] - d[2]*c[3] + d[3]*c[2];
			float t2 = d[0]*c[2] + d[1]*c[3] + d[2]*c[0] - d[3]*c[1];
			float t3 = d[0]*c[3] - d[1]*c[2] + d[2]*c[1] + d[3]*c[0];
			vector<float> t{ t0,t1,t2,t3 };
			// Save new quaternion
			currentQuaternion = t;
			// Reset variables
			axisRotation[0] = axisRotation[1] = axisRotation[2] = 0;
			currentPoint[0] = currentPoint[1] = 0;
			initialPoint[0] = initialPoint[1] = 0;
			currentAngle = 0;
			leftMouseActive = false;
			glutMouseFunc(NULL);
		}
	}
}

// Mouse movement CB
static void mouseMovCB(int x, int y) {
	if (leftMouseActive) {
		// Convert coordinates
		screenToPlaneCoords(x, y, currentPoint);
		// Save current angle
		currentAngle = computeAngle();
	}
}

// On windows reshape
static void reshapeCB(int width, int height) {
	if (width == 0 || height == 0) return;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	int deltaH = 0, deltaW = 0;
	if (width <= height) {
		/*deltaW = abs(width - height) / 2;

		C_HEIGHT = height;
		C_WIDTH = height;*/
		gluOrtho2D(-1.0, 1.0, -1.0*(float)height / (float)width, 1.0*(float)height / (float)width);
		C_EDGE = width;
	}
	else {
		/*deltaH = abs(height-width) / 2;
		C_HEIGHT = width;
		C_WIDTH = width;*/
		gluOrtho2D(-1.0*(float)width / (float)height, 1.0*(float)width / (float)height, -1.0, 1.0);
		C_EDGE = height;
	}
	glMatrixMode(GL_MODELVIEW);
	glViewport(0,0, width, height);

	
	//glFrustum(-1, 1, -1, 1, 1, 3);
}

// Other action submenu callback
static void otherSubMenuCB(int choice) {
	// Clear
	if (choice == 0) {
		cleared = true;
		drawAxis = leftMouseActive = false;
		// Reset variables
		currentQuaternion[0] = 1;
		currentQuaternion[1] = currentQuaternion[2] = currentQuaternion[3] = 0;
		axisRotation[0] = axisRotation[1] = axisRotation[2] = 0;
		currentPoint[0] = currentPoint[1] = 0;
		initialPoint[0] = initialPoint[1] = 0;
		currentAngle = 0;

		clearBothBuffers();
	}
	// Refresh
	else if (choice == 1) {
		cleared = false;
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
	if (choice == 0) {
		glutMouseFunc(mouseCB);
	}
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
	glutInitWindowSize(C_EDGE, C_EDGE);
	windowID = glutCreateWindow("Montoya5 assignement 2");
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	clearBothBuffers();

	// Initialize menus
	initMenus();

	// Event function
	glutDisplayFunc(displayCB);
	glutReshapeFunc(reshapeCB);
	glutIdleFunc(displayTea);
	glutMouseFunc(NULL);
	glutMotionFunc(mouseMovCB);


	// Glut loop start
	glutMainLoop();
	return 0;
}