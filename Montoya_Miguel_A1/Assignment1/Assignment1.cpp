// Assignment1.cpp
// Miguel Angel Montoya
//

#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <string>
#include <algorithm>
#include "utilities.h"
#include "GL/freeglut.h"
#include "Evpoly/Evpoly.h"

using namespace std;

// Window Parameters
static int C_WIDTH = 700;
static int C_HEIGHT = 700;
static int windowID;

static int N = 20; // Default samples
static pExp *userPoly; // Polynomial
static int drawingType = 1; // 0 = points 1= lines 2 = lines(From search)
static bool pointsDrawn = false; // Store if matrix points have been drawn
static bool linesDrawn = false; // Store if lines have been drawn
static bool linesSearchDrawn = false; // Store if lines from search have been drawn
static float **evaluationMatrix; // Store points values
static int getPointsCounter = 0; // 0 = no points are being stored, 1 = waiting for point 1, 2 = waiting for point 2, 3 = can do line
static vector<vector<float>> points; // Points of segment (Faster redraw on resize)
static vector<vector<float>> midpoints; // Midpoints of segment for curve search (Faster redraw on resize)
static vector<vector<float>> drawnVertexSearch; // Store vertex found on search (Faster redraw on resize)


//
// Callbacks ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

// Mouse pressed callback
static void getPoint(int button, int state, int x, int y) {
	// Skip non relevant clicks
	if (button == GLUT_RIGHT_BUTTON || state != GLUT_UP) {
		return;
	}
	// If first point need capture
	if (getPointsCounter == 1) {
		vector<float> p(3);
		// Adjust to viewport possible resize
		if (x > C_WIDTH) x = C_WIDTH;
		p[0] = (x - C_WIDTH / (float)2) / (C_WIDTH / (float)2);
		p[1] = (-1 * y + (glutGet(GLUT_WINDOW_HEIGHT) - C_HEIGHT) + C_HEIGHT / (float)2) / (C_HEIGHT /(float)2);
		// Evaluate point
		float eval = userPoly->eval(p[0], p[1]);
		p[2] = eval;
		cout << "p(" << p[0] << " " << p[1] << ")\tf(p) = " << p[2] << "\n";
		// Store point at first position
		points.push_back(p);
		// Draw point
		glutSwapBuffers();
		if (eval >= 0) {
			glColor3f(1.0, 1.0, 0.0);
			glPointSize(3.0);
			glBegin(GL_POINTS);
			glVertex2d(p[0], p[1]);
			glEnd();
		}
		else {
			glColor3f(1.0, 0.0, 0.0);
			glPointSize(3.0);
			glBegin(GL_POINTS);
			glVertex2d(p[0], p[1]);
			glEnd();
		}
		updateAndClean();
		// Request next point
		getPointsCounter = 2;
		cout << "Click on coordinate q\n";
	}
	// If second point needs capture
	else if (getPointsCounter == 2) {
		vector<float> q(3);
		// Adjust to viewport
		if (x > C_WIDTH) x = C_WIDTH;
		q[0] = ((float)x - (float)C_WIDTH / 2) / (C_WIDTH/2);
		q[1] = (-1 * y + (glutGet(GLUT_WINDOW_HEIGHT) - C_HEIGHT) + C_HEIGHT / (float)2) / (C_HEIGHT / (float)2);
		// Point evaluation
		float eval = userPoly->eval(q[0], q[1]);
		q[2] = eval;
		cout << "q(" << q[0] << " " << q[1] << ")\tf(q) = " << q[2] << "\n";
		// Put point in vector (Last position)
		points.push_back(q);
		// Draw point
		glutSwapBuffers();
		if (eval >= 0) {
			glColor3f(1.0, 1.0, 0.0);
			glPointSize(3.0);
			glBegin(GL_POINTS);
			glVertex2d(q[0], q[1]);
			glEnd();
		}
		else {
			glColor3f(1.0, 0.0, 0.0);
			glPointSize(3.0);
			glBegin(GL_POINTS);
			glVertex2d(q[0], q[1]);
			glEnd();
		}
		updateAndClean();
		// Request point checking
		getPointsCounter = 3;
	}
	// If all points have been captured and have different evaluation signs, find intersection.
	// If not, request second point again.
	if (getPointsCounter == 3) {
		if (signbit(points[0][2]) == signbit(points[points.size() - 1][2])) {
			getPointsCounter = 2;
			cout << "f(p) and f(q) have different signs. Please click on a new q.\n";
		}
		else {
			getPointsCounter = 0;
			cout << "Line segment intersects. Finding curve now.\n";
			findIntersection();
			glutMouseFunc(NULL);
		}
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
		drawingType = 0;
		clear();
		getPointsAll();
		drawPointsAll();
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
		refresh();
	}
	else if (choice == 2) {
		glutDestroyWindow(windowID);
		exit(0);
	}
}

// Evaluate submenu callback
static void evaluateSubMenuCB(int choice) {
	if (choice == 0) {
		clear();
		getPointsAll();
		drawLinesAll();
	}
	else if (choice == 1) {
		clear();
		getPointsCounter = 1; // Request first point;
		points.clear();
		midpoints.clear();
		drawnVertexSearch.clear();
		glutMouseFunc(getPoint);
		cout << "Click on coordinate p\n";
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
		cout << "Polynomial read: " << stringPoly << "\n";
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

// Resize callback
static void resize(int width, int height) {
	if (width < height) {
		C_WIDTH = width;
		C_HEIGHT = width;
		glViewport(0, 0, width, width);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(-1, 1, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		refresh();
	}
	else if(height < width){
		C_WIDTH = height;
		C_HEIGHT = height;
		glViewport(0, 0, height, height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(-1, 1, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		refresh();
	}
}

//
// Utility functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

// Screen functions ----------------------------------------------------
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
	linesSearchDrawn = false;
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawAxis();
	glutSwapBuffers();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

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

// Refresh (Change to size)
static void refresh() {
	if (pointsDrawn) {
		clear();
		drawPointsAll();
	}
	else if (linesDrawn) {
		clear();
		drawLinesAll();
	}
	else if (linesSearchDrawn) {
		clear();
		drawAxis();
		linesSearchDrawn = true;
		for (std::vector<vector<float>>::iterator it = points.begin(); it != points.end(); it++) {
			vector<float> point = *it;
			if (point[2] >= 0) {
				glColor3f(1.0, 1.0, 0.0);
				glPointSize(3.0);
				glBegin(GL_POINTS);
				glVertex2d(point[0], point[1]);
				glEnd();
			}
			else {
				glColor3f(1.0, 0.0, 0.0);
				glPointSize(3.0);
				glBegin(GL_POINTS);
				glVertex2d(point[0], point[1]);
				glEnd();
			}
		}
		for (std::vector<vector<float>>::iterator it = midpoints.begin(); it != midpoints.end(); it++) {
			vector<float> midpoint = *it;
			if (midpoint[2] >= 0) {
				glColor3f(1.0, 1.0, 0.0);
				drawLetter(midpoint[0], midpoint[1]);
			}
			else {
				glColor3f(1.0, 0.0, 0.0);
				drawLetter(midpoint[0], midpoint[1]);
			}
		}
		for (std::vector<vector<float>>::iterator it = drawnVertexSearch.begin(); it != drawnVertexSearch.end(); it++) {
			vector<float> currentVertexA = *it;
			glColor3f(1.0, 1.0, 1.0);
			glLineWidth(2.0);
			glBegin(GL_LINES);
			drawBoxLines(currentVertexA);
			glEnd();
		}
		updateAndClean();
	}
	else {
		clear();
		drawAxis();
	}
}

// Evaluation matrix (Points) functions --------------------------------
// Build evaluation matrix
static void buildEvalMatrix() {
	int size = N * 2 + 1;
	evaluationMatrix = new float*[size];
	for (int i = 0; i < size; i++) {
		evaluationMatrix[i] = new float[size];
	}
}

// Deletes evaluation matrix
static void deleteEvalMatrix(int prevN) {
	int size = prevN * 2 + 1;
	for (int i = 0; i < size; i++)
		delete[] evaluationMatrix[i];
	delete[] evaluationMatrix;
}

// Find curve from line created by user functions ----------------------
// Returns the coordinate of the A-vertex (as stated in the figure below) of the box which the coordinate belongs.
static void getVertexA(float x, float y, vector<float> &vertexA) {
	float xf, yf;
	if(x < 0) 	xf = x - ((1 / (float) N) - abs(fmod(x, 1 / (float) N)));
	else xf = x - abs(fmod(x, 1 / (float) N));
	if (y < 0) 	yf = y - ((1 / (float) N) - abs(fmod(y, 1 / (float) N)));
	else yf = y - abs(fmod(y, 1 / (float) N));
	vertexA[0] = xf;
	vertexA[1] = yf;
}

// Evaluate box using polynomial
static bool differentVertex(vector<float> &vertexA) {
	float fullStep = 1 / (float) N;
	// Get other 3 vertex
	float vertexB[2] = { vertexA[0], vertexA[1] + fullStep };
	float vertexC[2] = { vertexA[0] + fullStep, vertexA[1] };
	float vertexD[2] = { vertexA[0] + fullStep, vertexA[1] + fullStep };
	// Obtain evaluation 
	float evalA = userPoly->eval(vertexA[0], vertexA[1]);
	float evalB = userPoly->eval(vertexB[0], vertexB[1]);
	float evalC = userPoly->eval(vertexC[0], vertexC[1]);
	float evalD = userPoly->eval(vertexD[0], vertexD[1]);
	// Obtain evaluation signs
	bool signA = signbit(evalA);
	bool signB = signbit(evalB);
	bool signC = signbit(evalC);
	bool signD = signbit(evalD);
	
	// Return false if all equal sign
	return (!(signA == signB && signB == signC && signC == signD));
}

/*
Box logic
	B ----- D
	|		|
	|		|
	A ----- C
*/
// Draw line that passes trough box with vertexA
static void drawBoxLines(vector<float> &vertexA) {
	float fullStep = 1 / (float) N;

	// Get other 3 vertex
	vector<float> vertexB = { vertexA[0], vertexA[1] + fullStep, 0 };
	vector<float> vertexC = { vertexA[0] + fullStep, vertexA[1], 0 };
	vector<float> vertexD = { vertexA[0] + fullStep, vertexA[1] + fullStep, 0 };
	// Obtain evaluation 
	vertexB[2] = userPoly->eval(vertexB[0], vertexB[1]);
	vertexC[2] = userPoly->eval(vertexC[0], vertexC[1]);
	vertexD[2] = userPoly->eval(vertexD[0], vertexD[1]);
	// Obtain evaluation signs
	bool signA = signbit(vertexA[2]);
	bool signB = signbit(vertexB[2]);
	bool signC = signbit(vertexC[2]);
	bool signD = signbit(vertexD[2]);
	/*cout << "Drawing vA:" << vertexA[0] << "," << vertexA[1]
		<< "\nvB:" << vertexB[0] << ", " << vertexB[1]
		<< "\nvC:" << vertexC[0] << ", " << vertexC[1]
		<< "\nvD:" << vertexD[0] << ", " << vertexD[1] << "\n";
	cout << signA << " " << signB << " " << signC << " " << signD << "\n";*/
	
	// Check for cases of 1:3 different signs
	// Line from left to bottom
	if (signA != signB && signA != signC && signA != signD) {
		float leftIntrs = getLineCoordBox(vertexA[2], vertexB[2]);
		float botIntrs = getLineCoordBox(vertexA[2], vertexC[2]);
		glVertex2f(vertexA[0], vertexA[1] + leftIntrs);
		glVertex2f(vertexA[0] + botIntrs, vertexA[1]);
	}
	// Line from top to left
	else if (signB != signA && signB != signC && signB != signD) {
		float topIntrs = getLineCoordBox(vertexB[2], vertexD[2]);
		float leftIntrs = getLineCoordBox(vertexA[2], vertexB[2]);
		glVertex2f(vertexA[0] + topIntrs, vertexA[1] + fullStep);
		glVertex2f(vertexA[0], vertexA[1] + leftIntrs);
	}
	// Line from bottom to right
	else if (signC != signA && signC != signB && signC != signD) {
		float botIntrs = getLineCoordBox(vertexA[2], vertexC[2]);
		float rightIntrs = getLineCoordBox(vertexC[2], vertexD[2]);
		glVertex2f(vertexA[0] + botIntrs, vertexA[1]);
		glVertex2f(vertexA[0] + fullStep, vertexA[1] + rightIntrs);
	}
	// Line from top to right
	else if (signD != signA && signD != signB && signD != signC) {
		float topIntrs = getLineCoordBox(vertexB[2], vertexD[2]);
		float rightIntrs = getLineCoordBox(vertexC[2], vertexD[2]);
		glVertex2f(vertexA[0] + topIntrs, vertexA[1] + fullStep);
		glVertex2f(vertexA[0] + fullStep, vertexA[1] + rightIntrs);
	}
	// Check for cases of 2:2 different signs, if no case has been found
	// Line from left to right
	else if (signA == signC && signB == signD && signA != signD) {
		float leftIntrs = getLineCoordBox(vertexA[2], vertexB[2]);
		float rightIntrs = getLineCoordBox(vertexC[2], vertexD[2]);
		glVertex2f(vertexA[0], vertexA[1] + leftIntrs);
		glVertex2f(vertexA[0] + fullStep, vertexA[1] + rightIntrs);
	}
	// Line from top to bottom
	else if (signA == signB && signC == signD && signA != signD) {
		float topIntrs = getLineCoordBox(vertexB[2], vertexD[2]);
		float botIntrs = getLineCoordBox(vertexA[2], vertexC[2]);
		glVertex2f(vertexA[0] + topIntrs, vertexA[1] + fullStep);
		glVertex2f(vertexA[0] + botIntrs, vertexA[1]);
	}
	// Ambiguous Case
	else if (signA == signD && signB == signC && signA != signB) {
		glColor3f(1.0, 0.0, 1.0);
		float leftIntrs = getLineCoordBox(vertexA[2], vertexB[2]);
		float rightIntrs = getLineCoordBox(vertexC[2], vertexD[2]);
		glVertex2f(vertexA[0], vertexA[1] + leftIntrs);
		glVertex2f(vertexA[0] + fullStep, vertexA[1] + rightIntrs);
		float topIntrs = getLineCoordBox(vertexB[2], vertexD[2]);
		float botIntrs = getLineCoordBox(vertexA[2], vertexC[2]);
		glVertex2f(vertexA[0] + topIntrs, vertexA[1] + fullStep);
		glVertex2f(vertexA[0] + botIntrs, vertexA[1]);
	}
}

// Draw letter X
static void drawLetter(float x, float y) {
	char c = 'x';
	glRasterPos2d(x, y);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, c);
	glPopMatrix();
}

// Check if line generated by two points intersects the curve
static void findIntersection() {
	vector<float> p(3), q(3);
	p = points[0];
	q = points[points.size() - 1];
	// Get first midpoint for binary search
	vector<float> nextMidpoint{ (p[0] + q[0]) / 2, (p[1] + q[1]) / 2, 0 };
	nextMidpoint[2] = userPoly->eval(nextMidpoint[0], nextMidpoint[1]); // Evaluate midpoint
	vector<float> nextMidpointVertexA(2); // Get its correspondant vertexA
	getVertexA(nextMidpoint[0], nextMidpoint[1], nextMidpointVertexA);
	// While that vertexA's square doesn't have different vertex signs, continue binary search
	while (!differentVertex(nextMidpointVertexA)) {
		// Store midpoint
		midpoints.push_back(nextMidpoint);
		// Draw current midpoint
		if (nextMidpoint[2] >= 0) {
			glutSwapBuffers();
			glColor3f(1.0, 1.0, 0.0);
			drawLetter(nextMidpoint[0], nextMidpoint[1]);
			updateAndClean();
		}
		else {
			glutSwapBuffers();
			glColor3f(1.0, 0.0, 0.0);
			drawLetter(nextMidpoint[0], nextMidpoint[1]);
			updateAndClean();
		}
		// Get next midpoint in the "left" side
		if (signbit(p[2]) != signbit(nextMidpoint[2])) {
			vector<float> t{ (p[0] + nextMidpoint[0]) / 2, (p[1] + nextMidpoint[1]) / 2,  0 };
			t[2] = userPoly->eval(t[0], t[1]);
			q = nextMidpoint;
			nextMidpoint = t;
			getVertexA(nextMidpoint[0], nextMidpoint[1], nextMidpointVertexA);
		}
		// Get next midpoint in the "right" side
		else {
			vector<float> t{ (nextMidpoint[0] + q[0]) / 2, (nextMidpoint[1] + q[1]) / 2,  0 };
			t[2] = userPoly->eval(t[0], t[1]);
			p = nextMidpoint;
			nextMidpoint = t;
			getVertexA(nextMidpoint[0], nextMidpoint[1], nextMidpointVertexA);
		}
	}
	// Draw final midpoint
	if (nextMidpoint[2] >= 0) {
		glutSwapBuffers();
		glColor3f(1.0, 1.0, 0.0);
		drawLetter(nextMidpoint[0], nextMidpoint[1]);
		updateAndClean();
	}
	else {
		glutSwapBuffers();
		glColor3f(1.0, 0.0, 0.0);
		drawLetter(nextMidpoint[0], nextMidpoint[1]);
		updateAndClean();
	}
	cout << "Curve found. Searching curve's boxes.\nIf N > 25, process might take a few seconds for complex figures\n";
	findBoxes(nextMidpointVertexA);
}

// Find boxes from curve found, to draw lines
static void findBoxes(vector<float> &rootVertex) {
	linesSearchDrawn = true;
	drawingType = 2;

		rootVertex.push_back(userPoly->eval(rootVertex[0], rootVertex[1]));
	vector<vector<float>> stackVertexA; // Stack for DFS of A-type vertex
	drawnVertexSearch.clear(); // Alredy drawn (Explored) boxes, storing only their A-type vertex. Set to 0.
	stackVertexA.push_back(rootVertex);

	int iterations = 1;
	// DFS
	while (!stackVertexA.empty()) {
		if (iterations % 125 == 0) {
			cout << "Working...";
		}
		//cout << "STACK SIZE:" << stackVertexA.size() << "\n";
		vector<float> aVertex;
		aVertex = stackVertexA.back(); // Get first element of stack
		drawnVertexSearch.push_back(aVertex); // Push to explored vertex
		stackVertexA.pop_back(); // Pop from stack
		//cout << aVertex[0] << "," << aVertex[1] << "\tv" << aVertex[2] << "CURRENT\n";
		
		// Store complete box vertex
		vector<float> bVertex = { aVertex[0], aVertex[1] + 1 / (float) N, 0 }, // upper box aVertex is the current bVertex
			cVertex = { aVertex[0] + 1 / (float) N, aVertex[1], 0 }, // right box aVertex is the current cVertex
			dVertex = { aVertex[0] + 1 / (float) N, aVertex[1] + 1 / (float) N, 0 },
			bottomVertexA = { aVertex[0], aVertex[1] - 1 / (float) N, 0 }, // bottom box aVertex is the current aVertex minus a bottom step
			leftVertexA = { aVertex[0] - 1 / (float) N, aVertex[1], 0 }; // left box aVertex is the current aVertex minus a left step
		bVertex[2] = userPoly->eval(bVertex[0], bVertex[1]);
		cVertex[2] = userPoly->eval(cVertex[0], cVertex[1]);
		dVertex[2] = userPoly->eval(dVertex[0], dVertex[1]);
		bottomVertexA[2] = userPoly->eval(bottomVertexA[0], bottomVertexA[1]);
		leftVertexA[2] = userPoly->eval(leftVertexA[0], leftVertexA[1]);
		
		// Checking upper box. If in range, has different signs and isn't to be explored or alredy explored, add to queue.
		if (-1 <= bVertex[1] && bVertex[1] < 1
			&& (find(drawnVertexSearch.begin(), drawnVertexSearch.end(), bVertex) == drawnVertexSearch.end())
			&& (find(stackVertexA.begin(), stackVertexA.end(), bVertex) == stackVertexA.end())
			&& signbit(bVertex[2]) != signbit(dVertex[2])) { // Check current upper edge for sign differences
			//cout << bVertex[0] << "," << bVertex[1] << "FOUNDUP\n";
			stackVertexA.push_back(bVertex);
		}

		// Checking bottom box.
		if (bottomVertexA[1] >= -1 && bottomVertexA[1] < 1
			&& (find(drawnVertexSearch.begin(), drawnVertexSearch.end(), bottomVertexA) == drawnVertexSearch.end())
			&& (find(stackVertexA.begin(), stackVertexA.end(), bottomVertexA) == stackVertexA.end())
			&& signbit(aVertex[2]) != signbit(cVertex[2])) { // Check current bottom edge for sign differences
			//cout << bottomVertexA[0] << "," << bottomVertexA[1] << "FOUNDDOWN\n";
			stackVertexA.push_back(bottomVertexA);
		}
		// Checking right box
		if (-1 <= cVertex[1] && cVertex[0] < 1
			&& (find(drawnVertexSearch.begin(), drawnVertexSearch.end(), cVertex) == drawnVertexSearch.end())
			&& (find(stackVertexA.begin(), stackVertexA.end(), cVertex) == stackVertexA.end())
			&& signbit(cVertex[2]) != signbit(dVertex[2])) { // Check current right edge for sign differences
			//cout << cVertex[0] << "," << cVertex[1] << "FOUNDRIGHT\n";
			stackVertexA.push_back(cVertex);
		}
		// Checking left box.
		if (leftVertexA[0] >= -1 && leftVertexA[1] < 1
			&& (find(drawnVertexSearch.begin(), drawnVertexSearch.end(), leftVertexA) == drawnVertexSearch.end())
			&& (find(stackVertexA.begin(), stackVertexA.end(), leftVertexA) == stackVertexA.end())
			&& signbit(aVertex[2]) != signbit(bVertex[2])) { // Check current left edge for sign differences
			//cout << leftVertexA[0] << "," << leftVertexA[1] << "FOUNDLEFT\n";
			stackVertexA.push_back(leftVertexA);
		}
		iterations++;
	}
	drawAxis();
	glutSwapBuffers();
	glLineWidth(2.0);
	glBegin(GL_LINES);
	for (std::vector<vector<float>>::iterator it = drawnVertexSearch.begin(); it != drawnVertexSearch.end(); it++) {
		vector<float> currentVertexA = *it;
		glColor3f(1.0, 1.0, 1.0);
		drawBoxLines(currentVertexA);
	}
	glEnd();
	updateAndClean();
	cout << "Figure completed";
}

// Evaluate all points/boxes and create lines functions -----------------
// Calculate points
static void getPointsAll() {
	float i = -1;
	for (int a = 0; a <= 2 * N; i += 1 / (float) N, a++) {
		float j = -1;
		for (int b = 0; b <= 2 * N; j += 1 / (float) N, b++) {
			float eval = userPoly->eval(i, j);
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
	for (float i = -1; i <= 1; i += 1 / (float) N, a++) {
		int b = 0;
		for (float j = -1; j <= 1; j += 1 / (float) N, b++) {
			float eval = evaluationMatrix[a][b];
			// If point is positive, draw green
			if (eval >= 0) {
				glColor3f(1.0, 1.0, 0.0);
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

// Obtain with more exactitude start and end of line inside a box
static float getLineCoordBox(float vertex1, float vertex2) {
	// Proportional location depending to vertex value
	return (1 / (float) N) * abs(vertex1) / (abs(vertex1) + abs(vertex2));
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
	float i = -1; // Coordinate x
	// Itarate over evaluation matrix. Incrementig x-coord and matrix position
	for (int a = 0; a < 2*N; i += 1 / (float) N, a++) {
		float j = -1; // Coordinate y
		// Itarate over evaluation matrix. Incrementig y-coord and matrix position
		for (int b = 0; b < 2*N; j += 1 / (float) N, b++) {
			// Get vertex values
			float vertexA, vertexB, vertexC, vertexD;
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

			float fullStep = 1 / (float) N;
			// Check for cases of 1:3 different signs
			// Line from left to bottom
			if (signA != signB && signA != signC && signA != signD) {
				float leftIntrs = getLineCoordBox(vertexA, vertexB);
				float botIntrs = getLineCoordBox(vertexA, vertexC);
				glVertex2f(i, j + leftIntrs);
				glVertex2f(i + botIntrs, j);
			}
			// Line from top to left
			else if (signB != signA && signB != signC && signB != signD) {
				float topIntrs = getLineCoordBox(vertexB, vertexD);
				float leftIntrs = getLineCoordBox(vertexA, vertexB);
				glVertex2f(i + topIntrs, j + fullStep);
				glVertex2f(i, j + leftIntrs);
			}
			// Line from bottom to right
			else if (signC != signA && signC != signB && signC != signD) {
				float botIntrs = getLineCoordBox(vertexA, vertexC);
				float rightIntrs = getLineCoordBox(vertexC, vertexD);
				glVertex2f(i + botIntrs, j);
				glVertex2f(i + fullStep, j + rightIntrs);
			}
			// Line from top to right
			else if (signD != signA && signD != signB && signD != signC) {
				float topIntrs = getLineCoordBox(vertexB, vertexD);
				float rightIntrs = getLineCoordBox(vertexC, vertexD);
				glVertex2f(i + topIntrs, j + fullStep);
				glVertex2f(i + fullStep, j + rightIntrs);
			}
			// Check for cases of 2:2 different signs, if no case has been found
			// Line from left to right
			else if (signA == signC && signB == signD && signA != signD) {
				float leftIntrs = getLineCoordBox(vertexA, vertexB);
				float rightIntrs = getLineCoordBox(vertexC, vertexD);
				glVertex2f(i, j + leftIntrs);
				glVertex2f(i + fullStep, j + rightIntrs);
			}
			// Line from top to bottom
			else if (signA == signB && signC == signD && signA != signD) {
				float topIntrs = getLineCoordBox(vertexB, vertexD);
				float botIntrs = getLineCoordBox(vertexA, vertexC);
				glVertex2f(i + topIntrs, j + fullStep);
				glVertex2f(i + botIntrs, j);
			}
			// Ambiguous Case
			else if (signA == signD && signB == signC && signA != signB) {
				glColor3f(1.0, 0.0, 1.0);
				float leftIntrs = getLineCoordBox(vertexA, vertexB);
				float rightIntrs = getLineCoordBox(vertexC, vertexD);
				glVertex2f(i, j + leftIntrs);
				glVertex2f(i + fullStep, j + rightIntrs);
				float topIntrs = getLineCoordBox(vertexB, vertexD);
				float botIntrs = getLineCoordBox(vertexA, vertexC);
				glVertex2f(i + topIntrs, j + fullStep);
				glVertex2f(i + botIntrs, j);
			}
		}
	}
	glEnd();
	updateAndClean();
}


//
// Setup functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

// Window initialization
static void initScreen() {
	glutInitWindowSize(C_WIDTH, C_HEIGHT);
	glutInitWindowPosition(430, 30);

	// Create window
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	windowID = glutCreateWindow("Assignment 1 - Montoya");
	glutDisplayFunc(redisplay);
	glutReshapeFunc(resize);
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
	glutAddMenuEntry("Draw scalar field", 4);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//
// MAIN ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int main(int argc, char** argv) {
	cout << "Hello World!\n\n" << "Circle function is loaded by default\n";

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