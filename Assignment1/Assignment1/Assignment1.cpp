// Assignment1.cpp
// Miguel Angel Montoya
//

#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <algorithm>
#include "utilities.h"
#include "GL/freeglut.h"
#include "Evpoly/Evpoly.h"

using namespace std;

// Window Parameters
static int C_WIDTH = 700;
static int C_HEIGHT = 700;
static int windowID;

static double N = 10; // Default samples
static pExp *userPoly; // Polynomial
static int drawingType = 1; // 0 = points 1= lines 2 = lines(From search)
static bool pointsDrawn = false; // Store if matrix points have been drawn
static bool linesDrawn = false; // Store if lines have been drawn
static bool linesSearchDrawn = false; // Store if lines from search have been drawn
static double **evaluationMatrix; // Store points values
static int getPointsCounter = 0; // 0 = no points are being stored, 1 = waiting for point 1, 2 = waiting for point 2, 3 = can do line
static double points[4] = { 0,0,0,0 }; // Points
static vector<vector<double>> drawnVertexSearch; // Store vertex found on search (Faster redraw on resize)


//
// Callbacks ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

// Mouse pressed callback
static void getPoint(int button, int state, int x, int y) {
	// If first point need capture
	if (getPointsCounter == 1) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
			// Adjust to ortho
			if (x > C_WIDTH) x = C_WIDTH;
			points[0] = (x - C_WIDTH / (double)2) / (C_WIDTH / (double)2);
			points[1] = (-1 * y + (glutGet(GLUT_WINDOW_HEIGHT) - C_HEIGHT) + C_HEIGHT / (double)2) / (C_HEIGHT /(double)2);
			cout << "P(" << points[0] << " " << points[1] << ")\n";
			getPointsCounter = 2;
		}
	}
	// If second point needs capture
	else if (getPointsCounter == 2) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
			// Adjust to ortho
			if (x > C_WIDTH) x = C_WIDTH;
			points[2] = ((double)x - (double)C_WIDTH / 2) / (C_WIDTH/2);
			points[3] = (-1 * y + (glutGet(GLUT_WINDOW_HEIGHT) - C_HEIGHT) + C_HEIGHT / (double)2) / (C_HEIGHT / (double)2);
			cout << "P(" << points[2] << " " << points[3] << ")\n";
			getPointsCounter = 3;
		}
	}
	// If all points have been captured, find intersection
	if (getPointsCounter == 3) {
		getPointsCounter == 0;
		glutMouseFunc(NULL);
		bool intersects = findIntersection();
		if (intersects == true) {
			cout << "Done!\n";
		}
		else {
			cout << "No curve, try again\n";
			for (int i = 0; i < 4; i++) {
				points[i] = 0;
			}
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
		glColor3f(0.0, 1.0, 0.0);
		glLineWidth(2.0);
		glBegin(GL_LINES);
		glVertex2d(points[0], points[1]);
		glVertex2d(points[2], points[3]);
		glEnd();
		glPointSize(3.0);
		glBegin(GL_POINTS);
		glVertex2d(points[0], points[1]);
		glVertex2d(points[2], points[3]);
		glEnd();
		for (std::vector<vector<double>>::iterator it = drawnVertexSearch.begin(); it != drawnVertexSearch.end(); it++) {
			vector<double> currentVertexA = *it;
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

// Find curve from line created by user functions ----------------------
// Returns the coordinate of the A-vertex (as stated in the figure below) of the box which the coordinate belongs.
static void getVertexA(double x, double y, vector<double> &vertexA) {
	double xf, yf;
	if(x < 0) 	xf = x - ((1 / N) - abs(fmod(x, 1 / N)));
	else xf = x - abs(fmod(x, 1 / N));
	if (y < 0) 	yf = y - ((1 / N) - abs(fmod(y, 1 / N)));
	else yf = y - abs(fmod(y, 1 / N));
	vertexA[0] = xf;
	vertexA[1] = yf;
}

// Evaluate box using polynomial
static bool differentVertexPoly(vector<double> &vertexA) {
	double fullStep = 1 / N;
	// Get other 3 vertex
	double vertexB[2] = { vertexA[0], vertexA[1] + fullStep };
	double vertexC[2] = { vertexA[0] + fullStep, vertexA[1] };
	double vertexD[2] = { vertexA[0] + fullStep, vertexA[1] + fullStep };
	// Obtain evaluation 
	double evalA = userPoly->eval(vertexA[0], vertexA[1]);
	double evalB = userPoly->eval(vertexB[0], vertexB[1]);
	double evalC = userPoly->eval(vertexC[0], vertexC[1]);
	double evalD = userPoly->eval(vertexD[0], vertexD[1]);
	// Obtain evaluation signs
	bool signA = signbit(evalA);
	bool signB = signbit(evalB);
	bool signC = signbit(evalC);
	bool signD = signbit(evalD);
	
	// Return false if all equal sign
	return (!(signA == signB && signB == signC && signC == signD));
}

// Darw line that passes trough box with vertexA
static void drawBoxLines(vector<double> &vertexA) {
	double fullStep = 1 / N;

	double i = vertexA[0];
	double j = vertexA[1];
	// Get other 3 vertex
	double vertexB[2] = { vertexA[0], vertexA[1] + fullStep };
	double vertexC[2] = { vertexA[0] + fullStep, vertexA[1] };
	double vertexD[2] = { vertexA[0] + fullStep, vertexA[1] + fullStep };
	// Obtain evaluation 
	double evalA = userPoly->eval(vertexA[0], vertexA[1]);
	double evalB = userPoly->eval(vertexB[0], vertexB[1]);
	double evalC = userPoly->eval(vertexC[0], vertexC[1]);
	double evalD = userPoly->eval(vertexD[0], vertexD[1]);
	// Obtain evaluation signs
	bool signA = signbit(evalA);
	bool signB = signbit(evalB);
	bool signC = signbit(evalC);
	bool signD = signbit(evalD);
	//cout << "Drawing vA:" << vertexA[0] << "," << vertexA[1] << "\n";
	//cout << signA << " " << signB << " " << signC << " " << signD << "\n";
	
	// Check for cases of 1:3 different signs
	// Line from left to bottom
	if (signA != signB && signA != signC && signA != signD) {
		double leftIntrs = getLineCoordBox(evalA, evalB);
		double botIntrs = getLineCoordBox(evalA, evalC);
		glVertex2f(i, j + leftIntrs);
		glVertex2f(i + botIntrs, j);
	}
	// Line from top to left
	else if (signB != signA && signB != signC && signB != signD) {
		double topIntrs = getLineCoordBox(evalB, evalD);
		double leftIntrs = getLineCoordBox(evalA, evalB);
		glVertex2f(i + topIntrs, j + fullStep);
		glVertex2f(i, j + leftIntrs);
	}
	// Line from bottom to right
	else if (signC != signA && signC != signB && signC != signD) {
		double botIntrs = getLineCoordBox(evalA, evalC);
		double rightIntrs = getLineCoordBox(evalC, evalD);
		glVertex2f(i + botIntrs, j);
		glVertex2f(i + fullStep, j + rightIntrs);
	}
	// Line from top to right
	else if (signD != signA && signD != signB && signD != signC) {
		double topIntrs = getLineCoordBox(evalB, evalD);
		double rightIntrs = getLineCoordBox(evalC, evalD);
		glVertex2f(i + topIntrs, j + fullStep);
		glVertex2f(i + fullStep, j + rightIntrs);
	}
	// Check for cases of 2:2 different signs, if no case has been found
	// Line from left to right
	else if (signA == signC && signB == signD && signA != signD) {
		double leftIntrs = getLineCoordBox(evalA, evalB);
		double rightIntrs = getLineCoordBox(evalC, evalD);
		glVertex2f(i, j + leftIntrs);
		glVertex2f(i + fullStep, j + rightIntrs);
	}
	// Line from top to bottom
	else if (signA == signB && signC == signD && signA != signD) {
		double topIntrs = getLineCoordBox(evalB, evalD);
		double botIntrs = getLineCoordBox(evalA, evalC);
		glVertex2f(i + topIntrs, j + fullStep);
		glVertex2f(i + botIntrs, j);
	}
	// Parallels A (Left-Top and Bot-Right)
	else if (signA == signD && signB == signC && signA != signB && signA == true) {
		double topIntrs = getLineCoordBox(evalB, evalD);
		double leftIntrs = getLineCoordBox(evalA, evalB);
		glVertex2f(i + topIntrs, j + fullStep);
		glVertex2f(i, j + leftIntrs);
		double botIntrs = getLineCoordBox(evalA, evalC);
		double rightIntrs = getLineCoordBox(evalC, evalD);
		glVertex2f(i + botIntrs, j);
		glVertex2f(i + fullStep, j + rightIntrs);
	}
	// Parallels B (Left-Bot and Top-Right)
	else if (signA == signD && signB == signC && signA != signB && signA == false) {
		double topIntrs = getLineCoordBox(evalB, evalD);
		double rightIntrs = getLineCoordBox(evalC, evalD);
		glVertex2f(i + topIntrs, j + fullStep);
		glVertex2f(i + fullStep, j + rightIntrs);
		double leftIntrs = getLineCoordBox(evalA, evalB);
		double botIntrs = getLineCoordBox(evalA, evalC);
		glVertex2f(i, j + leftIntrs);
		glVertex2f(i + botIntrs, j);
	}
}

// Evaluate box using the line created by the user
static bool differentVertexSegment(vector<double> &vertexA, double m, double b) {
	double fullStep = 1 / N;
	// Get other 3 vertex
	double vertexB[2] = { vertexA[0], vertexA[1] + fullStep };
	double vertexC[2] = { vertexA[0] + fullStep, vertexA[1] };
	double vertexD[2] = { vertexA[0] + fullStep, vertexA[1] + fullStep };
	//cout << vertexA[0] << " " << vertexB[0] << " " << vertexC[0] << " " << vertexD[0] << "\n";
	//cout << vertexA[1] << " " << vertexB[1] << " " << vertexC[1] << " " << vertexD[1] << "\n";

	// Obtain evaluation signs
	bool evalA = signbit(m*vertexA[0] - vertexA[1] + b);
	bool evalB = signbit(m*vertexB[0] - vertexB[1] + b);
	bool evalC = signbit(m*vertexC[0] - vertexC[1] + b);
	bool evalD = signbit(m*vertexD[0] - vertexD[1] + b);
	//cout << (m*vertexA[0] - vertexA[1] + b) << "-" << evalA << "\n" <<
	//		(m*vertexB[0] - vertexB[1] + b) << "-" << evalB << "\n" << 
	//		(m*vertexC[0] - vertexC[1] + b) << "-" << evalC << "\n" << 
	//		(m*vertexD[0] - vertexD[1] + b) << "-" << evalD << "\n";

	// Return false if all equal sign
	return !(evalA == evalB && evalB == evalC && evalC == evalD);
}

// Check if line generated by two points intersects the curve
static bool findIntersection() {
	double x1, y1, x2, y2, m, b;
	// Linear equation building
	x1 = points[0];
	y1 = points[1];
	x2 = points[2];
	y2 = points[3];
	m = (y2 - y1) / (x2 - x1);
	b = y1 - m * x1;

	vector<double> nextVertexA(2); // Next A-type vertex of segment to evaluate
	vector<double> initialVertexA(2); // Initial A-type vertex of segment to evaluate
	vector<double> finalVertexA(2); // Final A-type vertex of segment to evaluate
	getVertexA(x1, y1, nextVertexA);
	getVertexA(x1, y1, initialVertexA);
	getVertexA(x2, y2, finalVertexA);
	vector<vector<double>> drawnVertexA; // Alredy drawn (Explored) boxes, storing only their A-type vertex
	// Going 
	while ((initialVertexA[0] <= nextVertexA[0] && nextVertexA[0] <= finalVertexA[0] && initialVertexA[1] <= nextVertexA[1] && nextVertexA[1] <= finalVertexA[1])
		|| (initialVertexA[0] <= nextVertexA[0] && nextVertexA[0] <= finalVertexA[0] && initialVertexA[1] >= nextVertexA[1] && nextVertexA[1] >= finalVertexA[1])
		|| (initialVertexA[0] >= nextVertexA[0] && nextVertexA[0] >= finalVertexA[0] && initialVertexA[1] <= nextVertexA[1] && nextVertexA[1] <= finalVertexA[1])
		|| (initialVertexA[0] >= nextVertexA[0] && nextVertexA[0] >= finalVertexA[0] && initialVertexA[1] >= nextVertexA[1] && nextVertexA[1] >= finalVertexA[1])) {
		
		drawnVertexA.push_back(nextVertexA); // Push to explored vertex
		// If not on correct box, try next
		if (differentVertexPoly(nextVertexA)) {
			glutSwapBuffers();
			glColor3f(0.0, 1.0, 0.0);
			glLineWidth(2.0);
			glBegin(GL_LINES);
			glVertex2d(x1, y1);
			glVertex2d(x2, y2);
			glEnd();
			glPointSize(3.0);
			glBegin(GL_POINTS);
			glVertex2d(x1, y1);
			glVertex2d(x2, y2);
			glEnd();
			updateAndClean();
			findBoxes(nextVertexA);
			return true;
		}
		else {
			// How will the next boxes be computaded according to line slope.
			double nextBoxStepX, nextBoxStepY;
			if (x1 >= x2) {
				nextBoxStepX = -1 / N;
				//cout << "Will decrease X\n";
			}
			else {
				nextBoxStepX = 1 / N;
				//cout << "Will increase X\n";
			}
			if (y1 >= y2) {
				nextBoxStepY = -1 / N;
				//cout << "Will decrease Y\n";
			}
			else {
				nextBoxStepY = 1 / N;
				//cout << "Will increase Y\n";
			}
			double xChange = nextVertexA[0] + nextBoxStepX;
			double yChange = nextVertexA[1] + nextBoxStepY;

			// Store complete next vertex coordinates
			vector<double> upBox = { nextVertexA[0], yChange },
				downBox = { nextVertexA[0], yChange },
				leftBox = { xChange, nextVertexA[1] },
				rightBox = { xChange, nextVertexA[1] };

			// Checking upBox. If in range, has different signs and hasn't been alredy explored, it's the next vertex.
			if (-1 <= yChange && yChange <= 1
				&& (find(drawnVertexA.begin(), drawnVertexA.end(), upBox) == drawnVertexA.end())
				&& differentVertexSegment(upBox, m,b)) {
				//cout << upBox[0] << "," << upBox[1] << "FOUNDUP\n";
				nextVertexA = upBox;
			}
			// Checking rightBox
			else if (-1 <= xChange && xChange <= 1
				&& (find(drawnVertexA.begin(), drawnVertexA.end(), rightBox) == drawnVertexA.end())
				&& differentVertexSegment(rightBox, m, b)) {
				//cout << rightBox[0] << "," << rightBox[1] << "FOUNDRIGHT\n";
				nextVertexA = rightBox;
			}
		}
	}
	return false;
}

// Find boxes from curve found, to draw lines
static void findBoxes(vector<double> &rootVertex) {
	deque<vector<double>> queueVertexA; // Queue for BFS of A-type vertex
	drawnVertexSearch.clear(); // Alredy drawn (Explored) boxes, storing only their A-type vertex. Set to 0.
	queueVertexA.push_back(rootVertex);
	drawAxis();
	cout << "CURVE FOUND! Wait for drawing please\n";
	glutSwapBuffers();
	glColor3f(1.0, 1.0, 1.0);
	glLineWidth(2.0);
	glBegin(GL_LINES);
	// BFS
	while (!queueVertexA.empty()) {
		vector<double> currentVertexA;
		currentVertexA = queueVertexA.front(); // Get first element of queue
		drawBoxLines(currentVertexA); // Draw current vertex
		drawnVertexSearch.push_back(currentVertexA); // Push to explored vertex
		queueVertexA.pop_front(); // Pop from queue
		
		// Compute next 4 A-type vertex
		double xPlus, xMinus, yPlus, yMinus;
		xPlus = currentVertexA[0] + 1 / N; // Increment on x (Right)
		xMinus = currentVertexA[0] - 1 / N; // Decrement on x (Left)
		yPlus = currentVertexA[1] + 1 / N; // Increment on y (Up)
		yMinus = currentVertexA[1] - 1 / N; // Decrement on y (Down)
		// Store complete next vertex coordinates
		vector<double> upBox = { currentVertexA[0], yPlus },
			downBox = { currentVertexA[0], yMinus },
			leftBox = { xMinus, currentVertexA[1] },
			rightBox = { xPlus, currentVertexA[1] };
		
		// Checking upBox. If in range, has different signs and isn't to be explored or alredy explored, add to queue.
		if (yPlus <= 1 
			&& (find(drawnVertexSearch.begin(), drawnVertexSearch.end(), upBox) == drawnVertexSearch.end())
			&& (find(queueVertexA.begin(), queueVertexA.end(), upBox) == queueVertexA.end())
			&& differentVertexPoly(upBox)) {
			//cout << upBox[0] << "," << upBox[1] << "FOUNDUP\n";
			queueVertexA.push_back(upBox);
		}
		// Checking downBox.
		if (yMinus >= -1 
			&& (find(drawnVertexSearch.begin(), drawnVertexSearch.end(), downBox) == drawnVertexSearch.end())
			&& (find(queueVertexA.begin(), queueVertexA.end(), downBox) == queueVertexA.end())
			&& differentVertexPoly(downBox)) {
			//cout << downBox[0] << "," << downBox[1] << "FOUNDDOWN\n";
			queueVertexA.push_back(downBox);
		}
		// Checking rightBox
		if (xPlus <= 1
			&& (find(drawnVertexSearch.begin(), drawnVertexSearch.end(), rightBox) == drawnVertexSearch.end())
			&& (find(queueVertexA.begin(), queueVertexA.end(), rightBox) == queueVertexA.end())
			&& differentVertexPoly(rightBox)) {
			//cout << rightBox[0] << "," << rightBox[1] << "FOUNDRIGHT\n";
			queueVertexA.push_back(rightBox);
		}
		// Checking leftBox.
		if (xMinus >= -1
			&& (find(drawnVertexSearch.begin(), drawnVertexSearch.end(), leftBox) == drawnVertexSearch.end())
			&& (find(queueVertexA.begin(), queueVertexA.end(), leftBox) == queueVertexA.end())
			&& differentVertexPoly(leftBox)) {
			//cout << leftBox[0] << "," << leftBox[1] << "FOUNDLEFT\n";
			queueVertexA.push_back(leftBox);
		}
	}
	glEnd();
	updateAndClean();
	linesSearchDrawn = true;
	drawingType = 2;
}

// Evaluate all points/boxes and create lines functions -----------------
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

// Obtain with more exactitude start and end of line inside a box
static double getLineCoordBox(double vertex1, double vertex2) {
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
				double leftIntrs = getLineCoordBox(vertexA, vertexB);
				double botIntrs = getLineCoordBox(vertexA, vertexC);
				glVertex2f(i, j + leftIntrs);
				glVertex2f(i + botIntrs, j);
			}
			// Line from top to left
			else if (signB != signA && signB != signC && signB != signD) {
				double topIntrs = getLineCoordBox(vertexB, vertexD);
				double leftIntrs = getLineCoordBox(vertexA, vertexB);
				glVertex2f(i + topIntrs, j + fullStep);
				glVertex2f(i, j + leftIntrs);
			}
			// Line from bottom to right
			else if (signC != signA && signC != signB && signC != signD) {
				double botIntrs = getLineCoordBox(vertexA, vertexC);
				double rightIntrs = getLineCoordBox(vertexC, vertexD);
				glVertex2f(i + botIntrs, j);
				glVertex2f(i + fullStep, j + rightIntrs);
			}
			// Line from top to right
			else if (signD != signA && signD != signB && signD != signC) {
				double topIntrs = getLineCoordBox(vertexB, vertexD);
				double rightIntrs = getLineCoordBox(vertexC, vertexD);
				glVertex2f(i + topIntrs, j + fullStep);
				glVertex2f(i + fullStep, j + rightIntrs);
			}
			// Check for cases of 2:2 different signs, if no case has been found
			// Line from left to right
			else if (signA == signC && signB == signD && signA != signD) {
				double leftIntrs = getLineCoordBox(vertexA, vertexB);
				double rightIntrs = getLineCoordBox(vertexC, vertexD);
				glVertex2f(i, j + leftIntrs);
				glVertex2f(i + fullStep, j + rightIntrs);
			}
			// Line from top to bottom
			else if (signA == signB && signC == signD && signA != signD) {
				double topIntrs = getLineCoordBox(vertexB, vertexD);
				double botIntrs = getLineCoordBox(vertexA, vertexC);
				glVertex2f(i + topIntrs, j + fullStep);
				glVertex2f(i + botIntrs, j);
			}
			// Parallels A (Left-Top and Bot-Right)
			else if (signA == signD && signB == signC && signA != signB && signA == true) {
				double topIntrs = getLineCoordBox(vertexB, vertexD);
				double leftIntrs = getLineCoordBox(vertexA, vertexB);
				glVertex2f(i + topIntrs, j + fullStep);
				glVertex2f(i, j + leftIntrs);
				double botIntrs = getLineCoordBox(vertexA, vertexC);
				double rightIntrs = getLineCoordBox(vertexC, vertexD);
				glVertex2f(i + botIntrs, j);
				glVertex2f(i + fullStep, j + rightIntrs);
			}
			// Parallels B (Left-Bot and Top-Right)
			else if (signA == signD && signB == signC && signA != signB && signA == false) {
				double topIntrs = getLineCoordBox(vertexB, vertexD);
				double rightIntrs = getLineCoordBox(vertexC, vertexD);
				glVertex2f(i + topIntrs, j + fullStep);
				glVertex2f(i + fullStep, j + rightIntrs);
				double leftIntrs = getLineCoordBox(vertexA, vertexB);
				double botIntrs = getLineCoordBox(vertexA, vertexC);
				glVertex2f(i, j + leftIntrs);
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
	glutAddMenuEntry("Draw escalar field", 4);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//
// MAIN ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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