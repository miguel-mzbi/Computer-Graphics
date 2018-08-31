// Assignment1.cpp
// Miguel Angel Montoya
//

#include <iostream>
#include <string>
#include <stdio.h>
#include "utilities.h"
#include "GL/freeglut.h"
#include "Evpoly/Evpoly.h"

using namespace std;

// Window Parameters
static int C_WIDTH = 700;
static int C_HEIGHT = 700;
static int windowID;

static double N = 20; // Default samples
static pExp *userPoly; // Polynomial
static int drawingType = 1; // 0 = points 1= lines
static bool pointsDrawn = false; // Store if matrix points have been drawn
static bool linesDrawn = false; // Store if lines have been drawn
static double **evaluationMatrix; // Store points values
static int getPoints = 0; // 0 = no points are being stored, 1 = waiting for point 1, 2 = waiting for point 2, 3 = can do line
static double points[4] = { 0,0,0,0 }; // Points


//
// Callbacks ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

// Mouse pressed callback
static void getPoint(int button, int state, int x, int y) {
	// If first point need capture
	if (getPoints == 1) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
			points[0] = ((double)x - (double)C_WIDTH / 2) / (C_WIDTH/2);
			points[1] = (-1 * (double)y + (double)C_HEIGHT / 2) / (C_HEIGHT/2);
			cout << "P(" << points[0] << " " << points[1] << ")\n";
			getPoints = 2;
		}
	}
	// If second point needs capture
	else if (getPoints == 2) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
			points[2] = ((double)x - (double)C_WIDTH / 2) / (C_WIDTH/2);
			points[3] = (-1 * (double)y + (double)C_HEIGHT / 2) / (C_HEIGHT/2);
			cout << "P(" << points[2] << " " << points[3] << ")\n";
			getPoints = 3;
		}
	}
	// If all points have been captured
	if (getPoints == 3) {
		getPoints == 0;
		glutMouseFunc(NULL);
	}
}

// Redisplay callback
static void redisplay() {
}

// Menu callback
static void rootMenuCB(int choice) {
	if (choice == 2) {
		int prevN = N;
		cout << "Previous N = " << N << "\n";
		bool check = false;
		string stringN;
		while (!check) {
			cout << "Write the new N. Must be an int bigger than 0.\n";
			getline(cin, stringN);
			N = stoi(stringN);
			if (N > 0) {
				check = true;
			}
		}
		// If N > pN, increase size of points Matrix
		if (N > prevN) {
			deleteEvalMatrix(prevN);
			buildEvalMatrix();
		}
		cout << "New N = " << N << "\n";
		cout << "Remember to evaluate all or find the curve after changing the N\n";
	}
	else if (choice == 4) {
		drawingType = !drawingType;
		if (drawingType == 1) {
			clear();
			drawLinesAll();
		}
		else if (drawingType == 0) {
			clear();
			drawPointsAll();
		}
	}
}

// Other action submenu callback
static void otherSubMenuCB(int choice) {
	// Clear
	if (choice == 0) {
		clear();
	}
	// Refresh
	else if (choice == 1) {
		if (pointsDrawn) {
			clear();
			drawPointsAll();
		}
		else if (linesDrawn) {
			clear();
			drawLinesAll();
		}
		else {
			clear();
			drawAxis();
		}
	}
	else if (choice == 2) {
		glutDestroyWindow(windowID);
		exit(0);
	}
}

// Evaluate submenu callback
static void evaluateSubMenuCB(int choice) {
	if (choice == 0) {
		getPointsAll();
		if (drawingType == 1) {
			drawLinesAll();
		}
		else if (drawingType == 0) {
			drawPointsAll();
		}
	}
	else if (choice == 1) {
		getPoints = 1; // Request first point;
		glutMouseFunc(getPoint);
	}
}

// Polynomial submenu callback
static void polySubMenuCB(int choice) {
	// If polynomial read was selected
	if (choice == 0) {
		// String for input
		string stringPoly;
		cout << "Write polynomial:\n" << "(Permissible syntax elements are the symbols x and y, float coefficients, exponents that are positive integers;\n"
			<< "operations are '+', '-', '*', '^'; plus parentheses.\n";
		getline(cin, stringPoly);
		// Convert string to char*
		char charPoly[1024];
		strncpy_s(charPoly, stringPoly.c_str(), sizeof(charPoly));
		// Actual polynomial reading
		userPoly = readExp(charPoly);
		cout << "Remember to evaluate to or find the curve to see the new polynomial\n";
	}
}

// Polynomial sample submenu callback
static void polySampleSubMenuCB(int choice) {
	if (choice >= 0 && choice <= 12) {
		// Convert string to char*
		char charPoly[1024];
		strncpy_s(charPoly, samples[choice], sizeof(charPoly));
		// Actual polynomial reading
		userPoly = readExp(charPoly);
		cout << "Remember to evaluate to or find the curve to see the new polynomial\n";
	}
}

//
// Utility functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

// Draw axis
static void drawAxis() {
	glLineWidth(1.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex2d(0, 1);
	glVertex2d(0, -1);
	glVertex2d(1, 0);
	glVertex2d(-1, 0);
	glEnd();
}

// Build evaluation matrix
static void buildEvalMatrix() {
	int size = N * 2 + 1;
	evaluationMatrix = new double*[size];
	for (int i = 0; i < size; i++) {
		evaluationMatrix[i] = new double[size];
	}
}

// Deletes evaluation matrix
static void deleteEvalMatrix(int prevN) {
	int size = prevN * 2 + 1;
	for (int i = 0; i < size; i++)
		delete[] evaluationMatrix[i];
	delete[] evaluationMatrix;
}

// Apply changes and clean working buffer
static void updateAndClean() {
	glutSwapBuffers();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Clear both buffers
static void clear() {
	pointsDrawn = false;
	linesDrawn = false;
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawAxis();
	glutSwapBuffers();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Calculate points
static void getPointsAll() {
	double i = -1;
	for (int a = 0; a <= 2 * N; i += 1 / N, a++) {
		double j = -1;
		for (int b = 0; b <= 2 * N; j += 1 / N, b++) {
			double eval = userPoly->eval(i, j);
			// Store evaluations in matrix
			evaluationMatrix[a][b] = eval;
		}
	}
}

// Draw points
static void drawPointsAll() {
	drawAxis();
	pointsDrawn = true;
	// Points setup
	glPointSize(2.0);
	glBegin(GL_POINTS);
	int a = 0;
	for (double i = -1; i <= 1; i += 1 / N, a++) {
		int b = 0;
		for (double j = -1; j <= 1; j += 1 / N, b++) {
			double eval = evaluationMatrix[a][b];
			// If point is positive, draw green
			if (eval >= 0) {
				glColor3f(0.0, 1.0, 0.0);
			}
			// If point is negative, draw red
			else {
				glColor3f(1.0, 0.0, 0.0);
			}
			// Draw point
			glVertex2f(i, j);
		}
	}
	glEnd();
	updateAndClean();
}

// Obtain with more exactitude start and end of line
static double getIntersection(double vertex1, double vertex2) {
	// Proportional location depending to vertex value
	return (1 / N) * abs(vertex1) / (abs(vertex1) + abs(vertex2));
}
// Draw lines
/*
Box logic
	B ----- D
	|		|
	|		|
	A ----- C
To determine trough which edge the line goes, we don't care about the value of the vertex, only which vertex have different sign.
To determine exacly where does it cross, we need the real values.
This functions draws evrything on the plane
*/
static void drawLinesAll() {
	drawAxis();
	linesDrawn = true;

	// Lines setup
	glColor3f(1.0, 1.0, 1.0);
	glLineWidth(2.0);
	glBegin(GL_LINES);

	// Main loop
	double i = -1; // Coordinate x
	// Itarate over evaluation matrix. Incrementig x-coord and matrix position
	for (int a = 0; a < 2*N; i += 1 / N, a++) {
		double j = -1; // Coordinate y
		// Itarate over evaluation matrix. Incrementig y-coord and matrix position
		for (int b = 0; b < 2*N; j += 1 / N, b++) {
			// Get vertex values
			double vertexA, vertexB, vertexC, vertexD;
			vertexA = evaluationMatrix[a][b];
			vertexB = evaluationMatrix[a][b + 1];
			vertexC = evaluationMatrix[a + 1][b];
			vertexD = evaluationMatrix[a + 1][b + 1];
			// Get their sign
			int signA, signB, signC, signD;
			signA = signbit(vertexA);
			signB = signbit(vertexB);
			signC = signbit(vertexC);
			signD = signbit(vertexD);

			// If all signs are equal, skip
			if (signA == signB && signB == signC && signC == signD) {
				continue;
			}

			double fullStep = 1 / N;
			// Check for cases of 1:3 different signs
			// Line from left to bottom
			if (signA != signB && signA != signC && signA != signD) {
				double leftIntrs = getIntersection(vertexA, vertexB);
				double botIntrs = getIntersection(vertexA, vertexC);
				glVertex2f(i, j + leftIntrs);
				glVertex2f(i + botIntrs, j);
			}
			// Line from top to left
			else if (signB != signA && signB != signC && signB != signD) {
				double topIntrs = getIntersection(vertexB, vertexD);
				double leftIntrs = getIntersection(vertexA, vertexB);
				glVertex2f(i + topIntrs, j + fullStep);
				glVertex2f(i, j + leftIntrs);
			}
			// Line from bottom to right
			else if (signC != signA && signC != signB && signC != signD) {
				double botIntrs = getIntersection(vertexA, vertexC);
				double rightIntrs = getIntersection(vertexC, vertexD);
				glVertex2f(i + botIntrs, j);
				glVertex2f(i + fullStep, j + rightIntrs);
			}
			// Line from top to right
			else if (signD != signA && signD != signB && signD != signC) {
				double topIntrs = getIntersection(vertexB, vertexD);
				double rightIntrs = getIntersection(vertexC, vertexD);
				glVertex2f(i + topIntrs, j + fullStep);
				glVertex2f(i + fullStep, j + rightIntrs);
			}
			// Check for cases of 2:2 different signs, if no case has been found
			// Line from left to right
			else if (signA == signC && signB == signD && signA != signD) {
				double leftIntrs = getIntersection(vertexA, vertexB);
				double rightIntrs = getIntersection(vertexC, vertexD);
				glVertex2f(i, j + leftIntrs);
				glVertex2f(i + fullStep, j + rightIntrs);
			}
			// Line from top to bottom
			else if (signA == signB && signC == signD && signA != signD) {
				double topIntrs = getIntersection(vertexB, vertexD);
				double botIntrs = getIntersection(vertexA, vertexC);
				glVertex2f(i + topIntrs, j + fullStep);
				glVertex2f(i + botIntrs, j);
			}
			// Parallels A (Left-Top and Bot-Right)
			else if (signA == signD && signB == signC && signA != signB && signA == true) {
				double topIntrs = getIntersection(vertexB, vertexD);
				double leftIntrs = getIntersection(vertexA, vertexB);
				glVertex2f(i + topIntrs, j + fullStep);
				glVertex2f(i, j + leftIntrs);
				double botIntrs = getIntersection(vertexA, vertexC);
				double rightIntrs = getIntersection(vertexC, vertexD);
				glVertex2f(i + botIntrs, j);
				glVertex2f(i + fullStep, j + rightIntrs);
			}
			// Parallels B (Left-Bot and Top-Right)
			else if (signA == signD && signB == signC && signA != signB && signA == false) {
				double topIntrs = getIntersection(vertexB, vertexD);
				double rightIntrs = getIntersection(vertexC, vertexD);
				glVertex2f(i + topIntrs, j + fullStep);
				glVertex2f(i + fullStep, j + rightIntrs);
				double leftIntrs = getIntersection(vertexA, vertexB);
				double botIntrs = getIntersection(vertexA, vertexC);
				glVertex2f(i, j + leftIntrs);
				glVertex2f(i + botIntrs, j);
			}
		}
	}
	glEnd();
	updateAndClean();
}


//
// Setup functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

// Window initialization
static void initScreen() {
	glutInitWindowSize(C_WIDTH, C_HEIGHT);
	glutInitWindowPosition(500, 30);

	// Create window
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	windowID = glutCreateWindow("Assignment 1 - Montoya");
	glutDisplayFunc(redisplay);
	//glutReshapeFunc(resize);
}

// Field parameters
static void init() {
	glClearColor(0, 0, 0, 0);
	glViewport(0, 0, C_WIDTH, C_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(-1, 1, -1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
}

// Menu creation
static void initMenus() {
	// Other action submenu
	int otherSM = glutCreateMenu(otherSubMenuCB);
	glutAddMenuEntry("Clear", 0);
	glutAddMenuEntry("Refresh", 1);
	glutAddMenuEntry("Exit", 2);
	// Evaluate action submenu
	int evaluateSM = glutCreateMenu(evaluateSubMenuCB);
	glutAddMenuEntry("Evaluate All", 0);
	glutAddMenuEntry("Find Curve", 1);
	// Polynomial action submenu
	int polySampleSM = glutCreateMenu(polySampleSubMenuCB);
	glutAddMenuEntry("Circle", 0);
	glutAddMenuEntry("Ellipse", 1);
	glutAddMenuEntry("Hyperbola", 2);
	glutAddMenuEntry("Semicubic Parabola", 3);
	glutAddMenuEntry("Alpha Curve", 4);
	glutAddMenuEntry("Touching Tacnode", 5);
	glutAddMenuEntry("Crossing Tacnode", 6);
	glutAddMenuEntry("Lemniscate", 7);
	glutAddMenuEntry("Rabbit Ears", 8);
	glutAddMenuEntry("Curved Lobe", 9);
	glutAddMenuEntry("Trefoil", 10);
	glutAddMenuEntry("Clover Leaf", 11);

	// Polynomial action submenu
	int polySM = glutCreateMenu(polySubMenuCB);
	glutAddMenuEntry("Read Polynomial", 0);
	glutAddSubMenu("Polynomial Sample", polySampleSM);
	// Menu setup
	glutCreateMenu(rootMenuCB);
	glutAddSubMenu("Other Action", otherSM);
	glutAddSubMenu("Evaluate", evaluateSM);
	glutAddMenuEntry("Grid N [0,1]", 2);
	glutAddSubMenu("Polynomial", polySM);
	glutAddMenuEntry("Switch points/lines", 4);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//
// MAIN ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int main(int argc, char** argv) {
	cout << "Hello World!\n\n" << "Please reload graph if window is resized\n";

	glutInit(&argc, argv);
	initScreen();
	init();
	initMenus();
	glutMouseFunc(NULL);
	drawAxis();
	updateAndClean();

	// Default to circle
	char charPoly[1024];
	strncpy_s(charPoly, samples[0], sizeof(charPoly));
	userPoly = readExp(charPoly);
	// Build point matrix (Empty)
	buildEvalMatrix();

	// Start glut loop
	glutMainLoop();

	return 0;
}