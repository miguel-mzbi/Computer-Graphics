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
const static int C_WIDTH = 700;
const static int C_HEIGHT = 700;
static int windowID;

static double pN = -1; // Previous samples
static double N = 5; // Samples
pExp *userPoly; // Polynomial
static int drawingType = 1; // 0 = points 1= lines
static bool pointsDrawn = false; // Store if matrix points have been drawn
static bool linesDrawn = false; // Store if lines have been drawn
static bool isEvaluated = false; // Store if matrix has computed values
static double **evaluationMatrix; // Store points values


//
// Utility functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

// Build evaluation matrix
static void buildEvalMatrix() {
	int size = N * 2 + 1;
	evaluationMatrix = new double*[size];
	for (int i = 0; i < size; i++) {
		evaluationMatrix[i] = new double[size];
	}
}

// Deletes evaluation matrix
static void deleteEvalMatrix() {
	int size = pN * 2 + 1;
	for (int i = 0; i < size; i++)
		delete[] evaluationMatrix[i];
	delete[] evaluationMatrix;
}

// Apply changes
static void flushAndSwap() {
	glFlush();
	glutSwapBuffers();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFlush();
}

// Clear
static void clear() {
	pointsDrawn = false;
	linesDrawn = false;
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFlush();
	glutSwapBuffers();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFlush();
	glutSwapBuffers();
}

// Calculate points
static void getPoints() {
	isEvaluated = true;
	int a = 0;
	for (double i = -1; i <= 1; i += 1 / N, a++) {
		int b = 0;
		for (double j = -1; j <= 1; j += 1 / N, b++) {
			double eval = userPoly->eval(i, j);
			evaluationMatrix[a][b] = eval;
		}
	}
}

// Draw points
static void drawPoints() {
	pointsDrawn = true;
	glPointSize(2.0);
	glBegin(GL_POINTS);
	int a = 0;
	for (double i = -1; i <= 1; i += 1 / N, a++) {
		int b = 0;
		for (double j = -1; j <= 1; j += 1 / N, b++) {
			double eval = evaluationMatrix[a][b];
			if (eval >= 0) {
				glColor3f(0.0, 1.0, 0.0);
			}
			else {
				glColor3f(1.0, 0.0, 0.0);
			}
			glVertex2f(i, j);
		}
	}
	glEnd();
	flushAndSwap();
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
*/
static void drawLines() {
	glPointSize(2.0);
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_POINTS);
	glVertex2f(-1 + 1 / N, -1 + 1 / N);
	glVertex2f(-1 + 2 / N, -1 + 3 / N);
	glVertex2f(-1 + 3 / N, -1 + 5 / N);

	glEnd();
	linesDrawn = true;
	int size = N * 2;
	glColor3f(1.0, 1.0, 1.0);
	glLineWidth(2.0);
	glBegin(GL_LINES);
	double i = -1;
	for (int a = 0; a < 2*N; i += 1 / N, a++) {
		double j = -1;
		for (int b = 0; b < 2*N; j += 1 / N, b++) {
			double vertexA, vertexB, vertexC, vertexD;
			// Get vertex values
			vertexA = evaluationMatrix[a][b];
			vertexB = evaluationMatrix[a][b + 1];
			vertexC = evaluationMatrix[a + 1][b];
			vertexD = evaluationMatrix[a + 1][b + 1];
			int signA, signB, signC, signD;
			// Get their sign
			signA = signbit(vertexA);
			signB = signbit(vertexB);
			signC = signbit(vertexC);
			signD = signbit(vertexD);

			// Check for cases of 1:3 different signs
			// Line from left to bottom
			double mid = 1 / (N * 2);
			double full = 1 / N;
			if (signA != signB && signA != signC && signA != signD) {
				glVertex2f(i, j + mid);
				glVertex2f(i + mid, j);
			}
			// Line from top to left
			else if (signB != signA && signB != signC && signB != signD) {
				glVertex2f(i + mid, j + full);
				glVertex2f(i, j + mid);
			}
			// Line from bottom to right
			else if (signC != signA && signC != signB && signC != signD) {
				glVertex2f(i + mid, j);
				glVertex2f(i + full, j + mid);
			}
			// Line from top to right
			else if (signD != signA && signD != signB && signD != signC) {
				glVertex2f(i + mid, j + full);
				glVertex2f(i + full, j + mid);
			}
			// Check for cases of 2:2 different signs, if no case has been found
			// Line from left to right
			else if (signA == signC && signB == signD && signA != signD) {
				glVertex2f(i, j + mid);
				glVertex2f(i + full, j + mid);
			}
			// Line from top to bottom
			else if (signA == signB && signC == signD && signA != signD) {
				glVertex2f(i + mid, j + full);
				glVertex2f(i + mid, j);
			}
			// Parallels A
			else if (signA == signD && signB == signC && signA != signB && signA == true) {
				glVertex2f(i + mid, j + full);
				glVertex2f(i, j + mid);
				glVertex2f(i + mid, j);
				glVertex2f(i + full, j + mid);
			}
			// Parallels B
			else if (signA == signD && signB == signC && signA != signB && signA == false) {
				glVertex2f(i + mid, j + full);
				glVertex2f(i + full, j + mid);
				glVertex2f(i, j + mid);
				glVertex2f(i + mid, j);
			}
		}
	}
	glEnd();
	flushAndSwap();
}

//
// Callbacks ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

// Redisplay callback
static void redisplay() {
	glutSwapBuffers();
}

// Menu callback
static void rootMenuCB(int choice) {
	if (choice == 2) {
		cout << "Previous N = " << N << endl;
		pN = N;
		bool check = false;
		string stringN;
		while (!check) {
			cout << "Write the new N. Must be an int bigger than 0." << endl << "If graphic doesn't refresh automatically, refresh manually." << endl;
			getline(cin, stringN);
			N = stoi(stringN);
			if (N > 0) {
				check = true;
			}
		}
		cout << "New N = " << N << endl;
		// If N wasn't changed, there is no need to delete and redo the matrix.
		if (pN != N) {
			deleteEvalMatrix();
			buildEvalMatrix();
			getPoints();
		}
		if (pointsDrawn) {
			clear();
			drawPoints();
		}
		else if (linesDrawn) {
			clear();
			drawLines();
		}
	}
	else if (choice == 4) {
		drawingType = !drawingType;
		if (drawingType == 1) {
			clear();
			drawLines();
		}
		else if (drawingType == 0) {
			clear();
			drawPoints();
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
			drawPoints();
		}
		else if (linesDrawn) {
			clear();
			drawLines();
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
		if (!isEvaluated) {
			getPoints();
		}
		if (drawingType == 1) {
			drawLines();
		}
		else if (drawingType == 0) {
			drawPoints();
		}
	}
	else if (choice == 1) {
	}
}

// Polynomial submenu callback
static void polySubMenuCB(int choice) {
	// If polynomial read was selected
	if (choice == 0) {
		// String for input
		string stringPoly;
		cout << "Write polynomial:" << endl << "(Permissible syntax elements are the symbols x and y, float coefficients, exponents that are positive integers;"
			 << endl << "operations are '+', '-', '*', '^'; plus parentheses." << endl;
		getline(cin, stringPoly);
		// Convert string to char*
		char charPoly[1024];
		strncpy_s(charPoly, stringPoly.c_str(), sizeof(charPoly));
		// Actual polynomial reading
		userPoly = readExp(charPoly);
		isEvaluated = false;
		cout << "Remember to evaluate to see the new polynomial" << endl;
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
		cout << "Remeber to evaluate to see the new polynomial" << endl;
		isEvaluated = false;
	}
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
}

// Field parameters
static void init() {
	// XY Coordinates
	glClearColor(0, 0, 0, 0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1, 1, -1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
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
	cout << "Hello World!\n" << endl << "Please reload graph if window is resized" << endl;

	glutInit(&argc, argv);
	initScreen();
	init();
	initMenus();
	buildEvalMatrix();

	// Start glut loop
	glutMainLoop();

	return 0;
}