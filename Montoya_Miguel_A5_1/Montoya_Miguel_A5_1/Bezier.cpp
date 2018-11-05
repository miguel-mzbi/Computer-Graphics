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
int forceUpdate = 0;

// Logic
int segNum = 1000; // Number of segment that will build the line
float deCastelT = 0.5f; // DeCasteljau algorithm t value to show. Default t = 0.5
int radioSelectedMode; // (0) Add control point, (1) Move control point, (2) Delete control point, (3) DeCasteljau algorithm
int drawCurve = 1;
int drawExtendedCurve = 0;
int drawControlPoly = 0;
int drawHull = 0;
extern GLUI_Button *decreaseButton;

vector<vector<int>> pascalTable = {
	{1}, // n=0
	{1, 1}, // n=1
	{1, 2, 1}, // n=2
	{1, 3, 3, 1}, // n=3
	{1, 4, 6, 4, 1}, // n=4
	{1, 5, 10, 10, 5, 1}, // n=5
	{1, 6, 15, 20, 15, 6, 1} // n=6
};

// Points
vector<vector<float>> history;
vector<vector<float>> controlPoints;
unsigned int maxNumberControls = 32;
int movingPoint = 0;
int selectedPoint = -1;
vector<float> originalPointPosition(2);

// Substitutions
int X = 0;
int Y = 1;

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|					   Utility 						|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

void screenToWorldCoords(int(&screen)[2], vector<float> &world) {
	int x, y, w, h;
	GLUI_Master.get_viewport_area(&x, &y, &w, &h);

	int deltaH = 0, deltaW = 0, ed = 0;
	if (w > h) ed = h;
	else ed = w;
	deltaH = abs(h - ed) / 2;
	deltaW = abs(w - ed) / 2;

	float wx = 2 * (((screen[X] - (x + deltaW)) / (float)ed) - 0.5f);
	float wy = -2 * (((screen[Y] - (y + deltaH)) / (float)ed) - 0.5f);
	world[X] = wx;
	world[Y] = wy;
}

void clearScreen() {
	controlPoints = vector<vector<float>>();
	forceUpdate = 1;
}

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|				Callback functions					|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

void drawPoints() {
	glPointSize(4.0f);
	glBegin(GL_POINTS);
	for (unsigned int i = 0; i < controlPoints.size(); i++) {
		if (i == 0) {
			glColor3f(0.0f, 1.0f, 0.0f);
		}
		else if (i == controlPoints.size() -1) {
			glColor3f(0.0f, 0.0f, 1.0f);
		}
		else {
			glColor3f(1.0f, 0.0f, 0.0f);
		}
		glVertex2f(controlPoints[i][X], controlPoints[i][Y]);
	}
	glEnd();
}

void drawControlPolygon() {
	if (controlPoints.size() < 3) return;
	glColor3f(0.65f, 0.65f, 0.65f);
	glBegin(GL_LINES);
	for (unsigned int i = 0; i < controlPoints.size() - 1; i++) {
		glVertex2f(controlPoints[i][X], controlPoints[i][Y]);
		glVertex2f(controlPoints[i + 1][X], controlPoints[i + 1][Y]);
	}
	glEnd();
}

void drawDeCasteljau(vector<vector<float>> &points, float t) {
	if (points.size() <= 1) {
		return;
	}
	else {
		glColor3f(1.0f, 1.0f, 0.0f);
		glBegin(GL_LINES);
		for (unsigned int i = 0; i < points.size() - 1; i++) {
			glVertex2f(points[i][X], points[i][Y]);
			glVertex2f(points[i + 1][X], points[i + 1][Y]);
		}
		glEnd();

		vector<vector<float>> newPoints(points.size() - 1);
		for (unsigned int i = 0; i < newPoints.size(); i++) {
			float x = (1 - t) * points[i][X] + t * points[i + 1][X];
			float y = (1 - t) * points[i][Y] + t * points[i + 1][Y];
			newPoints[i] = vector<float>{ x, y };
		}

		glPointSize(4.0f);
		glBegin(GL_POINTS);
		for (unsigned int i = 0; i < newPoints.size(); i++) {
			glColor3f(1.0f, 0.5f, 0.0f);
			glVertex2f(newPoints[i][X], newPoints[i][Y]);
		}
		glEnd();

		drawDeCasteljau(newPoints, t);
	}
}

int orientation(vector<float> &p, vector<float> &q, vector<float> &r) {
	float val = (q[Y] - p[Y]) * (r[X] - q[X]) - (q[X] - p[X]) * (r[Y] - q[Y]);
	if (val == 0) return 0; // Colinear 
	return (val > 0) ? 1 : 2; // Clock or counterclock wise 
}

void drawConvexHull() {
	int n = controlPoints.size();
	if (n < 3) return;
	vector<vector<float>> hull;

	int l = 0;
	for (int i = 1; i < n; i++) {
		if (controlPoints[i][X] < controlPoints[l][X]) {
			l = i;
		}
	}
	
	int p = l;
	int q;
	do {
		// Add current point to result 
		hull.push_back(controlPoints[p]);

		q = (p + 1) % n;
		for (int i = 0; i < n; i++) {
			// If i is more counterclockwise than q, then update q 
			if (orientation(controlPoints[p], controlPoints[i], controlPoints[q]) == 2) {
				q = i;
			}
		}

		p = q;

	} while (p != l);  // While not on first point 

	glColor3f(0.15f, 0.15f, 0.15f);
	glBegin(GL_POLYGON);
	for (unsigned int i = 0; i < hull.size(); i++) {
		glVertex2f(hull[i][X], hull[i][Y]);
	}
	glEnd();
}

void increaseOrder() {
	int nOrder = controlPoints.size() + 1;
	if (nOrder > (int)maxNumberControls) return;
	vector<vector<float>> newOrder(nOrder, vector<float>(2));
	for (int i = 0; i < nOrder; i++) {
		if (i == 0) {
			newOrder[i][X] = nOrder * controlPoints[i][X] / nOrder;
			newOrder[i][Y] = nOrder * controlPoints[i][Y] / nOrder;
		}
		else if (i == nOrder - 1) {
			newOrder[i][X] = i * controlPoints[i-1][X] / nOrder;
			newOrder[i][Y] = i * controlPoints[i-1][Y] / nOrder;
		}
		else {
			newOrder[i][X] = ((nOrder - i) * controlPoints[i][X] + i * controlPoints[i - 1][X]) / nOrder;
			newOrder[i][Y] = ((nOrder - i) * controlPoints[i][Y] + i * controlPoints[i - 1][Y]) / nOrder;
		}
	}
	history = controlPoints;
	controlPoints = newOrder;
	forceUpdate = 1;
}

void decreaseOrder() {
	controlPoints = history;
	forceUpdate = 1;
}

float binomial(int degree, int k) {
	while (degree >= (int)pascalTable.size()) {
		int currentSize = pascalTable.size();
		vector<int> row(currentSize + 1);
		row[0] = 1;
		for (int i = 1, prev = currentSize - 1; i < currentSize; i++) {
			row[i] = pascalTable[prev][i - 1] + pascalTable[prev][i];
		}
		row[currentSize] = 1;
		pascalTable.push_back(row);
	}
	return (float)pascalTable[degree][k];
}

void Bezier(int degree, float t, vector<float> &result) {
	float sumX = 0.0f;
	float sumY = 0.0f;
	for (int k = 0; k <= degree; k++) {
		float bin = binomial(degree, k);
		sumX += controlPoints[k][X] * bin * powf((1.0f - t), (float)(degree - k)) * powf(t, (float)k);
		sumY += controlPoints[k][Y] * bin * powf((1.0f - t), (float)(degree - k)) * powf(t, (float)k);
	}
	result[X] = sumX;
	result[Y] = sumY;
}

void drawBezier() {
	int degree = controlPoints.size()-1;
	if (segNum < 1 || degree < 1) {
		return;
	}
	float step = 1.0f / segNum;
	vector<vector<float>> vertices;
	for (float t = 0.0f; t <= 1.0f; t += step) {
		vector<float> result{ 0.0f, 0.0f };
		Bezier(degree, t, result);
		vertices.push_back(result);
	}
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	for (unsigned int i = 0; i < vertices.size() - 1; i++) {
		glVertex2f(vertices[i][X], vertices[i][Y]);
		glVertex2f(vertices[i + 1][X], vertices[i + 1][Y]);
	}
	glEnd();
}

void drawExtendedBezier() {
	int degree = controlPoints.size() - 1;
	if (segNum < 1 || degree < 1) {
		return;
	}
	float step = 1.0f / segNum;
	vector<vector<float>> vertices;
	float t = 0.0f;
	float mX = 0.0f;
	float mY = 0.0f;
	while (-1.01f <= mX && mX <= 1.01f
		&& -1.01f <= mY && mY <= 1.01f) {
		vector<float> result{ 0.0f, 0.0f };
		Bezier(degree, t, result);
		vertices.push_back(result);
		mX = result[X];
		mY = result[Y];
		t -= step;
	}
	glColor3f(0.4f, 0.4f, 0.4f);
	glBegin(GL_LINES);
	for (unsigned int i = 0; i < vertices.size() - 1; i++) {
		glVertex2f(vertices[i][X], vertices[i][Y]);
		glVertex2f(vertices[i + 1][X], vertices[i + 1][Y]);
	}
	glEnd();
	vertices = vector<vector<float>>(0);
	t = 1.0f;
	mX = 0.0f;
	mY = 0.0f;
	while (-1.01f <= mX && mX <= 1.01f
		&& -1.01f <= mY && mY <= 1.01f) {
		vector<float> result{ 0.0f, 0.0f };
		Bezier(degree, t, result);
		vertices.push_back(result);
		mX = result[X];
		mY = result[Y];
		t += step;
	}
	glBegin(GL_LINES);
	for (unsigned int i = 0; i < vertices.size() - 1; i++) {
		glVertex2f(vertices[i][X], vertices[i][Y]);
		glVertex2f(vertices[i + 1][X], vertices[i + 1][Y]);
	}
	glEnd();
}

void displayFunction() {
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFlush();

	if (drawHull) {
		drawConvexHull();
	}
	if (drawControlPoly) {
		drawControlPolygon();
	}
	if (drawExtendedCurve) {
		drawExtendedBezier();
	}
	if (radioSelectedMode == 3) {
		drawDeCasteljau(controlPoints, deCastelT);
	}
	if (drawCurve) {
		drawBezier();
	}
	drawPoints();

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
	if (!movingPoint && !forceUpdate) {
		return;
	}
	else {
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFlush();
		if (drawHull) {
			drawConvexHull();
		}
		if (drawControlPoly) {
			drawControlPolygon();
		}
		if (drawExtendedCurve) {
			drawExtendedBezier();
		}
		if (radioSelectedMode == 3) {
			drawDeCasteljau(controlPoints, deCastelT);
		}
		if (drawCurve) {
			drawBezier();
		}
		drawPoints();

		forceUpdate = 0;

		glFlush();
	}
}

void mouseFunction(int button, int state, int x, int y) {
	// If is not left button return
	if (button != GLUT_LEFT_BUTTON) return;
	if (state == GLUT_DOWN) {
		decreaseButton->disable();
		// Add new control point
		if (radioSelectedMode == 0 && controlPoints.size() < maxNumberControls) {
			int screen[] = { x, y };
			vector<float> world{ 0.0f, 0.0f };
			screenToWorldCoords(screen, world);
			if (-1.01f > world[X] || world[X] > 1.01f
				|| -1.01f > world[Y] || world[Y] > 1.01f) {
				return;
			}
			controlPoints.push_back(world);
		}
		else if (radioSelectedMode == 2 && controlPoints.size() > 0) {
			int screen[] = { x, y };
			vector<float> world{ 0.0f, 0.0f };
			screenToWorldCoords(screen, world);
			if (-1.01f > world[X] || world[X] > 1.01f
				|| -1.01f > world[Y] || world[Y] > 1.01f) {
				return;
			}
			for (unsigned int i = 0; i < controlPoints.size(); i++) {
				if (world[X] - 0.02f <= controlPoints[i][X] && controlPoints[i][X] <= world[X] + 0.02f
					&& world[Y] - 0.02f <= controlPoints[i][Y] && controlPoints[i][Y] <= world[Y] + 0.02f) {
					controlPoints.erase(controlPoints.begin() + i);
				}
			}
		}
		else if (radioSelectedMode == 1) {
			int screen[] = { x, y };
			vector<float> world{ 0.0f, 0.0f };
			screenToWorldCoords(screen, world);
			if (-1.01f > world[X] || world[X] > 1.01f
				|| -1.01f > world[Y] || world[Y] > 1.01f) {
				return;
			}
			selectedPoint = -1;
			for (unsigned int i = 0; i < controlPoints.size(); i++) {
				if (world[X] - 0.02f <= controlPoints[i][X] && controlPoints[i][X] <= world[X] + 0.02f
					&& world[Y] - 0.02f <= controlPoints[i][Y] && controlPoints[i][Y] <= world[Y] + 0.02f) {
					movingPoint = 1;
					originalPointPosition[X] = controlPoints[i][X];
					originalPointPosition[Y] = controlPoints[i][Y];
					selectedPoint = i;
				}
			}
		}
	}
	else if (state == GLUT_UP) {
		if (radioSelectedMode == 1 && selectedPoint >= 0) {
			int screen[] = { x, y };
			vector<float> world{ 0.0f, 0.0f };
			screenToWorldCoords(screen, world);
			if (-1.01f > world[X] || world[X] > 1.01f
				|| -1.01f > world[Y] || world[Y] > 1.01f) {
				controlPoints[selectedPoint][X] = originalPointPosition[X];
				controlPoints[selectedPoint][Y] = originalPointPosition[Y];
			}
			else {
				controlPoints[selectedPoint][X] = world[X];
				controlPoints[selectedPoint][Y] = world[Y];
			}
			movingPoint = 0;
			selectedPoint = -1;
		}
	}
}

void motionFunction(int x, int y) {
	if (movingPoint) {
		int screen[] = { x, y };
		vector<float> world{ 0.0f, 0.0f };
		screenToWorldCoords(screen, world);
		controlPoints[selectedPoint][X] = world[X];
		controlPoints[selectedPoint][Y] = world[Y];
	}
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
	gluOrtho2D(-1.01, 1.01, -1.01, 1.01);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Set up callbacks (All done by GLUI, except motion)
	glutMotionFunc(motionFunction);

	// Create GUI
	buildGLUI(windowID);

	// Start loop
	glutMainLoop();

	return 0;
}