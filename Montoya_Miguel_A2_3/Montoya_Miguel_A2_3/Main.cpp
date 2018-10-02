// Montoya_Miguel_A2_2.cpp : This file contains the 'main' function. Program execution begins and ends there.
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

static const int NONE = -1;
static const int GLOBAL_ROTATION = 0;
static const int LOCAL_ROTATION = 1;
static const int TRANSLATION = 2;
static const int FOV = 3;

static int C_EDGE = 700;
static const float PI = atanf(1) * 4;

static float teaSize = 0.10f; // Teapot size

static bool drawAxis = false; // Draw axis if requested
static bool drawTeapot = true; // Draw axis if requested
static float cleared = false; // If screen is cleared, nothing will be shown
static int leftMouseActive = NONE; // If true, mouse movement is active. Depends on type.

static vector<float> initialPoint(2); // Stores point P
static vector<float> currentPoint(2); // Stores point P' when moving mouse

// Field of view
static float currentFOV = 30;
static float deltaFOV = 0;

// Local rotation
static float currentAngle = 0; // Stores angles of rotation according to mouse drag
static vector<float> currentQuaternion{ 1, 0, 0, 0 }; // Q(q, i, j, k) Stores quaternion applied to image shown on window
static vector<float> axisRotation{ 0.0, 0.0, 0.0 }; // Stores axis of rotation

// Global rotation
static float currentAngleGlobal = 0; // Stores angles of rotation according to mouse drag
static vector<float> currentQuaternionGlobal{ 1, 0, 0, 0 }; // Q(q, i, j, k) Stores quaternion applied to image shown on window
static vector<float> axisRotationGlobal{ 0.0, 0.0, 0.0 }; // Stores axis of rotation

// Translation
static vector<float> teapotCoords(3); // Teapot location
static vector<float> teapotTempCoords(3); // Teapot location

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

//static void rotateVector(vector<float> &vect, vector<float> &result) {
//	vector<float> quat{ currentQuaternionGlobal[0], currentQuaternionGlobal[1], currentQuaternionGlobal[2], currentQuaternionGlobal[3] };
//	// Create inverse quaternion
//	float l = 1 / (quat[0] * quat[0] + quat[1] * quat[1] + quat[2] * quat[2] + quat[3] * quat[3]);
//	vector<float> quatI{ l*quat[0], -l * quat[1], -l * quat[2], -l * quat[3] };
//	// Convert angle-axis system to quaternion, applying k to axis components, and storing the angle in radians and as cos(phi/2)
//	vector<float> v{ 0, vect[0], vect[1], vect[2] };
//
//	// Q' x V
//	float t0 = quatI[0] * v[0] - quatI[1] * v[1] - quatI[2] * v[2] - quatI[3] * v[3];
//	float t1 = quatI[0] * v[1] + quatI[1] * v[0] - quatI[2] * v[3] + quatI[3] * v[2];
//	float t2 = quatI[0] * v[2] + quatI[1] * v[3] + quatI[2] * v[0] - quatI[3] * v[1];
//	float t3 = quatI[0] * v[3] - quatI[1] * v[2] + quatI[2] * v[1] + quatI[3] * v[0];
//	vector<float> qv{ t0,t1,t2,t3 };
//
//
//	// Q'V x Q
//	float vP0 = qv[0] * quat[0] - qv[1] * quat[1] - qv[2] * quat[2] - qv[3] * quat[3];
//	float vP1 = qv[0] * quat[1] + qv[1] * quat[0] - qv[2] * quat[3] + qv[3] * quat[2];
//	float vP2 = qv[0] * quat[2] + qv[1] * quat[3] + qv[2] * quat[0] - qv[3] * quat[1];
//	float vP3 = qv[0] * quat[3] - qv[1] * quat[2] + qv[2] * quat[1] + qv[3] * quat[0];
//	vector<float> vP{ vP0,vP1,vP2,vP3 };
//	result = vP;
//}

// Display function for teapot drawing
static void displayTea() {
	if (cleared) return;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(currentFOV - deltaFOV, 1, -1.0, 1.0);
	glTranslatef(0, 0, -2);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// Rotate teapot (GLOBAL) according to current quaternion
	// Quaternion must be translated to axis-angle system using arc-cos(phi)*2, and then convert to degrees from radians
	glRotatef((180 / PI)*(acos(currentQuaternionGlobal[0])) * 2, currentQuaternionGlobal[1], currentQuaternionGlobal[2], currentQuaternionGlobal[3]);

	// If left mouse is active, draw axis of rotation and rotate (Global)
	if (leftMouseActive == GLOBAL_ROTATION) {
		glBegin(GL_LINES);
		glColor3f(1, 1, 0);
		glVertex3f(axisRotationGlobal[0], axisRotationGlobal[1], axisRotationGlobal[2]);
		glVertex3f(0, 0, 0);
		glEnd();
		glRotatef(currentAngleGlobal, axisRotationGlobal[0], axisRotationGlobal[1], axisRotationGlobal[2]);
	}

	// TRANSLATE
	vector<float> result1(4);
	//rotateVector(teapotCoords, result1);
	//glTranslatef(result1[1], result1[2], result1[3]);
	glTranslatef(teapotCoords[0], teapotCoords[1], teapotCoords[2]);
	if (leftMouseActive == TRANSLATION) {
		//vector<float> result2(4);
		//rotateVector(teapotTempCoords, result2);
		//glTranslatef(result2[1], result2[2], result2[3]);
		glTranslatef(teapotTempCoords[0], teapotTempCoords[1], teapotTempCoords[2]);
	}


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
		// Blue z axis
		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 1.0f);
		glEnd();
	}


	// Rotate teapot (LOCAL) according to current quaternion
	// Quaternion must be translates to axis-angle system using arc-cos(phi)*2, and then convert to degrees from radians
	glRotatef((180 / PI)*(acos(currentQuaternion[0])) * 2, currentQuaternion[1], currentQuaternion[2], currentQuaternion[3]);

	// If left mouse is active, draw axis of rotation and rotate
	if (leftMouseActive == LOCAL_ROTATION) {
		glBegin(GL_LINES);
		glColor3f(1, 1, 0);
		glVertex3f(axisRotation[0], axisRotation[1], axisRotation[2]);
		glVertex3f(0, 0, 0);
		glEnd();
		glRotatef(currentAngle, axisRotation[0], axisRotation[1], axisRotation[2]);
	}

	if (drawTeapot) {
		// Draw white teapot
		glColor3f(1.0, 1.0, 1.0);
		glutSolidTeapot(teaSize);
	}
	draw();
}

// Translate pixel coordinates to point in z=0 plane
static void screenToPlaneCoords(int x, int y, vector<float> &result) {
	int currentWidth = glutGet(GLUT_WINDOW_WIDTH);
	int currentHeight = glutGet(GLUT_WINDOW_HEIGHT);
	// Adjust coordinates to border created by resizing
	int borderX = 0, borderY = 0;
	if (currentWidth <= currentHeight) {
		borderY = (currentHeight - currentWidth) / 2;
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
// Local rotation
static float computeAngleLocal() {
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

	float distToAxis = (xR*a - yR * b) / sqrt(a*a + b * b); // Current mouse distance to axis.
	float currentDistanceAxis = (xI*a - yI * b) / sqrt(a*a + b * b); // Distance from initial point to axis.

	if (currentDistanceAxis - distToAxis >= 0) {
		return abs(currentDistanceAxis - distToAxis) * 360 / 2;
	}
	else {
		return -(abs(currentDistanceAxis - distToAxis) * 360 / 2);
	}
}

// Computes angle for temporal rotation, according to perpendicular distance of the mouse to the line.
// Global rotation
static float computeAngleGlobal() {
	float yR = currentPoint[1];
	float xR = currentPoint[0];
	float yI = initialPoint[1];
	float xI = initialPoint[0];

	// Get rid of division by 0
	float a, b;
	if (axisRotationGlobal[0] == 0) {
		a = 0;
	}
	else {
		a = 1 / axisRotationGlobal[0];
	}
	if (axisRotationGlobal[1] == 0) {
		b = 0;
	}
	else {
		b = 1 / axisRotationGlobal[1];
	}

	float distToAxis = (xR*a - yR * b) / sqrt(a*a + b * b); // Current mouse distance to axis.
	float currentDistanceAxis = (xI*a - yI * b) / sqrt(a*a + b * b); // Distance from initial point to axis.

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

// Mouse pressed CB for translation
static void mouseTranslationCB(int button, int state, int x, int y) {
	// Skip non relevant clicks
	if (button == GLUT_RIGHT_BUTTON) {
		return;
	}
	else {
		if (state == GLUT_DOWN) {
			if (glutGetModifiers() == GLUT_ACTIVE_CTRL) {
				leftMouseActive = FOV;
			}
			else {
				leftMouseActive = TRANSLATION;
			}
			screenToPlaneCoords(x, y, initialPoint);
		}
		// On mouse up, add last rotation permanently to current quaterion.
		else if (state == GLUT_UP) {
			if (leftMouseActive == FOV) {
				cout << currentFOV << " | " << deltaFOV << " | ";
				currentFOV -= deltaFOV;
				cout << currentFOV << "\n";
				deltaFOV = 0;
			}
			else {
				//vector<float>res(4);
				//rotateVector(currentQuaternionGlobal, res);
				teapotCoords[0] += teapotTempCoords[0];
				teapotCoords[1] += teapotTempCoords[1];
				teapotCoords[2] += teapotTempCoords[2];
			}

			leftMouseActive = NONE;
			glutMouseFunc(mouseTranslationCB);
		}
	}
}

// Mouse pressed CB for LOCAL rotation
static void mouseRotationLocalCB(int button, int state, int x, int y) {
	// Skip non relevant clicks
	if (button == GLUT_RIGHT_BUTTON) {
		return;
	}
	else {
		if (state == GLUT_DOWN) {
			// Returns coordinates for vector CP
			screenToPlaneCoords(x, y, initialPoint);
			// Creates axis of rotation and compensates for teapot location (When dragged). This ensures 90° when creating the axis of rotation
			axisRotation[0] = initialPoint[1];
			axisRotation[1] = -(initialPoint[0]);
			axisRotation[2] = 0;


			leftMouseActive = LOCAL_ROTATION;
			displayTea();
		}
		// On mouse up, add last rotation permanently to current quaterion.
		else if (state == GLUT_UP) {
			// Calculate K scalar for vector axis (normalize). z component of axis is not considered, as its always 0.
			float k = sin((currentAngle*PI) / (180 * 2.0f)) / sqrt(axisRotation[0] * axisRotation[0] + axisRotation[1] * axisRotation[1]);
			// Copy current quaternion
			vector<float> c{ currentQuaternion[0], currentQuaternion[1], currentQuaternion[2], currentQuaternion[3] };
			// Convert angle-axis system to quaternion, applying k to axis components, and storing the angle in radians and as cos(phi/2)
			vector<float> d{ cos((currentAngle*PI) / (180 * 2.0f)), k*axisRotation[0], k*axisRotation[1], k*axisRotation[2] };
			// Quaternion multiplication
			float t0 = d[0] * c[0] - d[1] * c[1] - d[2] * c[2] - d[3] * c[3];
			float t1 = d[0] * c[1] + d[1] * c[0] - d[2] * c[3] + d[3] * c[2];
			float t2 = d[0] * c[2] + d[1] * c[3] + d[2] * c[0] - d[3] * c[1];
			float t3 = d[0] * c[3] - d[1] * c[2] + d[2] * c[1] + d[3] * c[0];
			vector<float> t{ t0,t1,t2,t3 };
			// Save new quaternion
			currentQuaternion = t;
			// Reset variables
			axisRotation[0] = axisRotation[1] = axisRotation[2] = 0;
			currentPoint[0] = currentPoint[1] = 0;
			initialPoint[0] = initialPoint[1] = 0;
			currentAngle = 0;
			leftMouseActive = NONE;
			glutMouseFunc(mouseTranslationCB);
			displayTea();
		}
	}
}

// Mouse pressed CB for GLOBAL rotation
static void mouseRotationGlobalCB(int button, int state, int x, int y) {
	// Skip non relevant clicks
	if (button == GLUT_RIGHT_BUTTON) {
		return;
	}
	else {
		if (state == GLUT_DOWN) {
			// Returns coordinates for vector CP
			screenToPlaneCoords(x, y, initialPoint);
			// Creates axis of rotation and compensates for teapot location (When dragged). This ensures 90° when creating the axis of rotation
			axisRotationGlobal[0] = initialPoint[1] - teapotCoords[1];
			axisRotationGlobal[1] = -(initialPoint[0] - teapotCoords[0]);
			axisRotationGlobal[2] = 0;

			leftMouseActive = GLOBAL_ROTATION;
			displayTea();
		}
		// On mouse up, add last rotation permanently to current quaterion.
		else if (state == GLUT_UP) {
			// Calculate K scalar for vector axis (normalize). z component of axis is not considered, as its always 0.
			float k = sin((currentAngleGlobal*PI) / (180 * 2.0f)) / sqrt(axisRotationGlobal[0] * axisRotationGlobal[0] + axisRotationGlobal[1] * axisRotationGlobal[1]);
			// Copy current quaternion
			vector<float> c{ currentQuaternionGlobal[0], currentQuaternionGlobal[1], currentQuaternionGlobal[2], currentQuaternionGlobal[3] };
			// Convert angle-axis system to quaternion, applying k to axis components, and storing the angle in radians and as cos(phi/2)
			vector<float> d{ cos((currentAngleGlobal*PI) / (180 * 2.0f)), k*axisRotationGlobal[0], k*axisRotationGlobal[1], k*axisRotationGlobal[2] };

			// Quaternion multiplication
			float t0 = d[0] * c[0] - d[1] * c[1] - d[2] * c[2] - d[3] * c[3];
			float t1 = d[0] * c[1] + d[1] * c[0] - d[2] * c[3] + d[3] * c[2];
			float t2 = d[0] * c[2] + d[1] * c[3] + d[2] * c[0] - d[3] * c[1];
			float t3 = d[0] * c[3] - d[1] * c[2] + d[2] * c[1] + d[3] * c[0];
			vector<float> t{ t0,t1,t2,t3 };
			// Save new quaternion
			currentQuaternionGlobal = t;
			// Reset variables
			axisRotationGlobal[0] = axisRotationGlobal[1] = axisRotationGlobal[2] = 0;
			currentPoint[0] = currentPoint[1] = 0;
			initialPoint[0] = initialPoint[1] = 0;
			currentAngleGlobal = 0;
			leftMouseActive = NONE;
			glutMouseFunc(mouseTranslationCB);
			displayTea();

		}
	}
}

// Mouse movement CB
static void mouseMovCB(int x, int y) {
	if (leftMouseActive == LOCAL_ROTATION) {
		// Convert coordinates
		screenToPlaneCoords(x, y, currentPoint);
		// Save current angle
		currentAngle = computeAngleLocal();
	}
	else if (leftMouseActive == GLOBAL_ROTATION) {
		// Convert coordinates
		screenToPlaneCoords(x, y, currentPoint);
		// Save current angle
		currentAngleGlobal = computeAngleGlobal();
	}
	else if (leftMouseActive == TRANSLATION) {
		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
			// Convert coordinates
			screenToPlaneCoords(x, y, currentPoint);
			vector<float> deltaTranslation{ currentPoint[0] - initialPoint[0], currentPoint[1] - initialPoint[1], 0 };
			teapotTempCoords[0] = deltaTranslation[0];
			teapotTempCoords[1] = deltaTranslation[1];
			teapotTempCoords[2] = deltaTranslation[2];
		}
		else if (glutGetModifiers() == GLUT_ACTIVE_ALT) {
			// Convert coordinates
			screenToPlaneCoords(x, y, currentPoint);
			vector<float> deltaTranslation{ currentPoint[0] - initialPoint[0], currentPoint[1] - initialPoint[1], 0 };
			teapotTempCoords[0] = deltaTranslation[2];
			teapotTempCoords[1] = deltaTranslation[1];
			teapotTempCoords[2] = deltaTranslation[0];
		}

	}
	else if (leftMouseActive == FOV && glutGetModifiers() == GLUT_ACTIVE_CTRL) {
		screenToPlaneCoords(x, y, currentPoint);
		float deltaX = currentPoint[0] - initialPoint[0];
		float deltaY = currentPoint[1] - initialPoint[1];

		if (signbit(deltaY)) {
			deltaFOV = -sqrtf(deltaX*deltaX + deltaY * deltaY) * 4;
		}
		else {
			deltaFOV = +sqrtf(deltaX*deltaX + deltaY * deltaY) * 4;
		}

	}
	displayTea();

}

// On windows reshape
static void reshapeCB(int width, int height) {
	if (width == 0 || height == 0) return;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	int deltaH = 0, deltaW = 0;
	if (width <= height) {
		deltaH = abs(width - height) / 2;
		//gluOrtho2D(-1.0, 1.0, -1.0*(float)height / (float)width, 1.0*(float)height / (float)width);
		C_EDGE = width;
	}
	else {
		deltaW = abs(height - width) / 2;
		//gluOrtho2D(-1.0*(float)width / (float)height, 1.0*(float)width / (float)height, -1.0, 1.0);
		C_EDGE = height;
	}

	glMatrixMode(GL_MODELVIEW);
	glViewport(deltaW, deltaH, C_EDGE, C_EDGE);

}

// Other action submenu callback
static void otherSubMenuCB(int choice) {
	// Clear
	if (choice == 0) {
		cleared = true;
		// Reset variables
		leftMouseActive = NONE;
		drawAxis = drawTeapot = false;
		currentQuaternion[0] = 1;
		currentQuaternion[1] = currentQuaternion[2] = currentQuaternion[3] = 0;
		axisRotation[0] = axisRotation[1] = axisRotation[2] = 0;
		currentPoint[0] = currentPoint[1] = 0;
		initialPoint[0] = initialPoint[1] = 0;
		teapotCoords[0] = teapotCoords[1] = teapotCoords[2] = 0;
		teapotTempCoords[0] = teapotTempCoords[1] = teapotTempCoords[2] = 0;
		currentAngle = 0;
		deltaFOV = 0;
		currentFOV = 30;

		clearBothBuffers();
	}
	// Refresh
	else if (choice == 1) {
		cleared = false;
		drawTeapot = true;
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
		// Reset variables
		leftMouseActive = NONE;

		currentQuaternion[0] = 1;
		currentQuaternion[1] = currentQuaternion[2] = currentQuaternion[3] = 0;
		axisRotation[0] = axisRotation[1] = axisRotation[2] = 0;

		currentPoint[0] = currentPoint[1] = 0;
		initialPoint[0] = initialPoint[1] = 0;

		currentQuaternionGlobal[0] = 1;
		currentQuaternionGlobal[1] = currentQuaternionGlobal[2] = currentQuaternionGlobal[3] = 0;
		axisRotationGlobal[0] = axisRotationGlobal[1] = axisRotationGlobal[2] = 0;

		currentAngle = currentAngleGlobal = 0;

		teapotCoords[0] = teapotCoords[1] = teapotCoords[2] = 0;
		teapotTempCoords[0] = teapotTempCoords[1] = teapotTempCoords[2] = 0;

		clearBothBuffers();
		displayTea();
	}
	else if (choice == 1) {
		glutMouseFunc(mouseRotationGlobalCB);
	}
	else if (choice == 2) {
		glutMouseFunc(mouseRotationLocalCB);
	}
}

// Diplay submenu callback
static void displaySubMenuCB(int choice) {
	if (choice == 0) {
		drawAxis = !drawAxis;
		displayTea();
	}
	if (choice == 2) {
		drawTeapot = !drawTeapot;
		displayTea();
	}
}

// Lights submenu callback
static void lightSubMenuCB(int choice) {

}

void initMenus() {
	// Other action submenu
	int otherSM = glutCreateMenu(otherSubMenuCB);
	glutAddMenuEntry("Clear", 0);
	glutAddMenuEntry("Refresh", 1);
	glutAddMenuEntry("Exit", 2);
	// Rotate action submenu
	int rotateSM = glutCreateMenu(rotateSubMenuCB);
	glutAddMenuEntry("Reset view", 0);
	glutAddMenuEntry("Rotate global", 1);
	glutAddMenuEntry("Rotate local", 2);
	// Display action submenu
	int displaySM = glutCreateMenu(displaySubMenuCB);
	glutAddMenuEntry("Show axes (ON/OFF)", 0);
	glutAddMenuEntry("Auto-rotate (ON/OFF) NOT IMPLEMENTED", 1);
	glutAddMenuEntry("Show teapot (ON/OFF)", 2);
	glutAddMenuEntry("Show file object(s) (ON/OFF) NOT IMPLEMENTED", 3);

	int lightSM = glutCreateMenu(lightSubMenuCB);
	glutAddMenuEntry("Light 0 toggle", 0);
	glutAddMenuEntry("Light 1 toggle", 1);
	glutAddMenuEntry("Light 2 toggle", 2);
	glutAddMenuEntry("Light 3 toggle", 3);
	glutAddMenuEntry("Smooth shade (ON/OFF)", 3);

	// Menu setup
	glutCreateMenu(NULL);
	glutAddSubMenu("Other Action", otherSM);
	glutAddSubMenu("Rotate", rotateSM);
	glutAddSubMenu("Display", displaySM);
	glutAddSubMenu("Lights", lightSM);

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
	glutDisplayFunc(displayTea);
	glutReshapeFunc(reshapeCB);
	glutIdleFunc(NULL);
	glutMouseFunc(mouseTranslationCB);
	glutMotionFunc(mouseMovCB);


	// Glut loop start
	glutMainLoop();
	return 0;
}