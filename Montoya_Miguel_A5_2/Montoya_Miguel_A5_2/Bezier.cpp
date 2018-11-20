#include <iostream>
#include <vector>
#include <windows.h>
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
int mainWindowID;
int secWindowID;
int mainEdge = 500;
int secEdge = 300;
extern GLUI *glui;
int forceUpdateMain = 0;
int forceUpdateSec = 0;

// Curve Basics
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
unsigned int maxNumberControls = 32;
int movingPoint = 0;
int selectedPoint = -1;
vector<float> originalPointPosition(2);

// Weights
vector<float> weightsHistory;
int selectedWeightModify = -1;
int movingBar = 0;
int selectedHoverBar = -1;

// Curve 0
vector<vector<float>> controlPointsCurve0;
vector<float> weightsCurve0;
int segNumC0 = 1000;
float deCastelTC0 = 0.5f;
int radioSelectedModeC0 = 0;
int drawCurveC0 = 1;
int drawExtendedCurveC0 = 0;
int drawControlPolyC0 = 0;
int drawHullC0 = 0;

// Curve 1
vector<vector<float>> controlPointsCurve1;
vector<float> weightsCurve1;
int segNumC1 = 1000;
float deCastelTC1 = 0.5f;
int radioSelectedModeC1 = 0;
int drawCurveC1 = 1;
int drawExtendedCurveC1 = 0;
int drawControlPolyC1 = 0;
int drawHullC1 = 0;

// Colors
COLORREF customColors[16];
COLORREF colorGrid = RGB(25, 25, 25);
COLORREF colorBackground = RGB(0, 0, 0);
COLORREF colorControlPointC0 = RGB(255, 255, 0);
COLORREF colorControlPointC1 = RGB(255, 255, 0);
COLORREF colorControlPointHoveringC0 = RGB(255, 0, 0);
COLORREF colorControlPointHoveringC1 = RGB(255, 0, 0);
COLORREF colorControlPolyC0 = RGB(166, 166, 166);
COLORREF colorControlPolyC1 = RGB(166, 166, 166);
COLORREF colorDeCastelLinesC0 = RGB(0, 0, 255);
COLORREF colorDeCastelLinesC1 = RGB(0, 0, 255);
COLORREF colorDeCastelPointsC0 = RGB(0, 150, 200);
COLORREF colorDeCastelPointsC1 = RGB(0, 150, 200);
COLORREF colorDeCastelMainC0 = RGB(255, 128, 0);
COLORREF colorDeCastelMainC1 = RGB(255, 128, 0);
COLORREF colorConvexC0 = RGB(38, 38, 38);
COLORREF colorConvexC1 = RGB(38, 38, 38);
COLORREF colorBezierC0 = RGB(0, 255, 0);
COLORREF colorBezierC1 = RGB(0, 255, 0);
COLORREF colorExtendedC0 = RGB(102, 102, 102);
COLORREF colorExtendedC1 = RGB(102, 102, 102);

// Logic, initialized by default as curve 0
int selectedCurve = 0; // Selected curve to operate (0 or 1)
int *segNum = new int(segNumC0); // Number of segment that will build the line
float *deCastelT = new float(deCastelTC0); // DeCasteljau algorithm t value to show. Default t = 0.5
int *radioSelectedMode = new int(radioSelectedModeC0); // (0) Add control point, (1) Move control point, (2) Delete control point, (3) DeCasteljau algorithm
int *drawCurve = new int(drawCurveC0);
int *drawExtendedCurve = new int(drawExtendedCurveC0);
int *drawControlPoly = new int(drawControlPolyC0);
int *drawHull = new int(drawHullC0);

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

void screenToWorldCoordsSecondary(int(&screen)[2], vector<float> &world) {
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);

	int deltaH = 0, deltaW = 0, ed = 0;
	if (width > height) ed = height;
	else ed = width;
	deltaH = abs(height - ed) / 2;
	deltaW = abs(width - ed) / 2;

	float wx = 2 * (((screen[X] - deltaW) / (float)ed) - 0.5f);
	float wy = -2 * (((screen[Y] - deltaH) / (float)ed) - 0.5f);
	world[X] = wx;
	world[Y] = wy;
}

void clearScreen() {
	controlPointsCurve0 = vector<vector<float>>();
	controlPointsCurve1 = vector<vector<float>>();
	weightsCurve0 = vector<float>();
	weightsCurve1 = vector<float>();
	forceUpdateMain = 1;
	forceUpdateSec = 1;
}

void applyChanges() {
	if (selectedCurve == 1) {
		deCastelTC1 = *deCastelT;
		radioSelectedModeC1 = *radioSelectedMode;
		drawCurveC1 = *drawCurve;
		drawExtendedCurveC1 = *drawExtendedCurve;
		drawControlPolyC1 = *drawControlPoly;
		drawHullC1 = *drawHull;
		segNumC1 = *segNum;
	}
	else {
		deCastelTC0 = *deCastelT;
		radioSelectedModeC0 = *radioSelectedMode;
		drawCurveC0 = *drawCurve;
		drawExtendedCurveC0 = *drawExtendedCurve;
		drawControlPolyC0 = *drawControlPoly;
		drawHullC0 = *drawHull;
		segNumC0 = *segNum;
	}
}

void changeCurve() {
	if (selectedCurve == 1) {
		*deCastelT = deCastelTC1;
		*radioSelectedMode = radioSelectedModeC1;
		*drawCurve = drawCurveC1;
		*drawExtendedCurve = drawExtendedCurveC1;
		*drawControlPoly = drawControlPolyC1;
		*drawHull = drawHullC1;
		*segNum = segNumC1;

	}
	else {
		*deCastelT = deCastelTC0;
		*radioSelectedMode = radioSelectedModeC0;
		*drawCurve = drawCurveC0;
		*drawExtendedCurve = drawExtendedCurveC0;
		*drawControlPoly = drawControlPolyC0;
		*drawHull = drawHullC0;
		*segNum = segNumC0;
	}

	forceUpdateSec = 1;
}

void setColor(COLORREF color) {
	glColor3f(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
}

void drawGrid() {
	setColor(colorGrid);
	glBegin(GL_LINES);
	for (float i = -1.0f; i < 1.1f; i += 0.1f) {
		glVertex2f(-1.0f, i);
		glVertex2f(1.0f, i);
		glVertex2f(i, -1.0f);
		glVertex2f(i, 1.0f);
	}
	glEnd();
}

float distance(vector<float> &a, vector<float> &b) {
	return sqrt(pow(a[X] - b[X], 2) + pow(a[Y] - b[Y], 2));
}

COLORREF getColor(COLORREF prev) {
	CHOOSECOLOR cc;
	ZeroMemory(&cc, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.lpCustColors = (LPDWORD)customColors;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	cc.rgbResult = prev;

	if (ChooseColor(&cc) == TRUE) {
		return cc.rgbResult;
	}
	return NULL;
}

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|				Bezier functions					|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

void drawPoints() {
	glPointSize(4.0f);
	glBegin(GL_POINTS);
	for (unsigned int i = 0; i < controlPointsCurve0.size(); i++) {
		if (selectedCurve == 0 && (i == selectedWeightModify || i == selectedHoverBar)) {
			setColor(colorControlPointHoveringC0);
		}
		else {
			setColor(colorControlPointC0);
		}
		glVertex2f(controlPointsCurve0[i][X], controlPointsCurve0[i][Y]);
	}
	glEnd();

	glPointSize(4.0f);
	glBegin(GL_POINTS);
	for (unsigned int i = 0; i < controlPointsCurve1.size(); i++) {
		if (selectedCurve == 0 && (i == selectedWeightModify || i == selectedHoverBar)) {
			setColor(colorControlPointHoveringC1);
		}
		else {
			setColor(colorControlPointC1);
		}
		glVertex2f(controlPointsCurve1[i][X], controlPointsCurve1[i][Y]);
	}
	glEnd();
}

void drawControlPolygon(int curveToDraw) {
	vector<vector<float>> controlPoints;
	if (curveToDraw == 0) {
		controlPoints = controlPointsCurve0;
		setColor(colorControlPolyC0);
	}
	else {
		controlPoints = controlPointsCurve1;
		setColor(colorControlPolyC1);
	}

	if (controlPoints.size() < 3) return;
	glBegin(GL_LINES);
	for (unsigned int i = 0; i < controlPoints.size() - 1; i++) {
		glVertex2f(controlPoints[i][X], controlPoints[i][Y]);
		glVertex2f(controlPoints[i + 1][X], controlPoints[i + 1][Y]);
	}
	glEnd();
}

void drawDeCasteljau(vector<vector<float>> &points, vector<float> &weights, float t, int curveToDraw) {
	if (points.size() <= 1) {
		return;
	}
	else {
		// Draw lines
		if (curveToDraw == 0) setColor(colorDeCastelLinesC0);
		else setColor(colorDeCastelLinesC1);
		glBegin(GL_LINES);
		for (unsigned int i = 0; i < points.size() - 1; i++) {
			glVertex2f(points[i][X], points[i][Y]);
			glVertex2f(points[i + 1][X], points[i + 1][Y]);
		}
		glEnd();
		// Get next weights and points
		vector<vector<float>> newPoints(points.size() - 1);
		vector<float> newWeights(weights.size() - 1);
		for (unsigned int i = 0; i < newPoints.size(); i++) {
			newWeights[i] = (1 - t) * weights[i] + t * weights[i + 1];
			float x = (1 - t) * weights[i] * points[i][X] / newWeights[i] + t * weights[i + 1] * points[i + 1][X] / newWeights[i];
			float y = (1 - t) * weights[i] * points[i][Y] / newWeights[i] + t * weights[i + 1] * points[i + 1][Y] / newWeights[i];
			newPoints[i] = vector<float>{ x, y };
		}
		// Draw points
		if (curveToDraw == 0 && newPoints.size() > 1) setColor(colorDeCastelPointsC0);
		else if (curveToDraw == 1 && newPoints.size() > 1) setColor(colorDeCastelPointsC1);
		else if (curveToDraw == 0 && newPoints.size() == 1) setColor(colorDeCastelMainC0);
		else if (curveToDraw == 1 && newPoints.size() == 1) setColor(colorDeCastelMainC1);
		glPointSize(4.0f);
		glBegin(GL_POINTS);
		for (unsigned int i = 0; i < newPoints.size(); i++) {
			glVertex2f(newPoints[i][X], newPoints[i][Y]);
		}
		glEnd();

		drawDeCasteljau(newPoints, newWeights, t, curveToDraw);
	}
}

int orientation(vector<float> &p, vector<float> &q, vector<float> &r) {
	float val = (q[Y] - p[Y]) * (r[X] - q[X]) - (q[X] - p[X]) * (r[Y] - q[Y]);
	if (val == 0) return 0; // Colinear 
	return (val > 0) ? 1 : 2; // Clock or counterclock wise 
}

void drawConvexHull(int curveToDraw) {
	vector<vector<float>> controlPoints;
	if (curveToDraw == 0) {
		controlPoints = controlPointsCurve0;
	}
	else {
		controlPoints = controlPointsCurve1;
	}

	int n = controlPoints.size();
	if (n < 3) return;
	vector<vector<float>> hull;
	// Get left-most point
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

	if (curveToDraw == 0) setColor(colorConvexC0);
	else setColor(colorConvexC1);
	glBegin(GL_POLYGON);
	for (unsigned int i = 0; i < hull.size(); i++) {
		glVertex2f(hull[i][X], hull[i][Y]);
	}
	glEnd();
}

void increaseOrder() {
	vector<vector<float>> controlPoints;
	if (selectedCurve == 0) {
		controlPoints = controlPointsCurve0;
	}
	else {
		controlPoints = controlPointsCurve1;
	}

	int nOrder = controlPoints.size() + 1;
	if (nOrder > (int)maxNumberControls || nOrder < 3) {
		decreaseButton->disable();
		return;
	}
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
	if (selectedCurve == 0) {
		controlPointsCurve0 = newOrder;
		weightsCurve0.push_back(1.0f);
		weightsHistory = weightsCurve0;
	}
	else {
		controlPointsCurve1 = newOrder;
		weightsCurve1.push_back(1.0f);
		weightsHistory = weightsCurve1;
	}
	forceUpdateMain = 1;
	forceUpdateSec = 1;
}

void decreaseOrder() {
	if (selectedCurve == 0) {
		controlPointsCurve0 = history;
		weightsCurve0 = weightsHistory;
	}
	else {
		controlPointsCurve1 = history;
		weightsCurve1 = weightsHistory;
	}
	forceUpdateMain = 1;
	forceUpdateSec = 1;
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

void Bezier(int degree, float t, vector<float> &result, vector<vector<float>> &controlPoints, int curveToDraw) {
	vector<float> weightControls;
	if (curveToDraw == 0) {
		weightControls = weightsCurve0;
	}
	else {
		weightControls = weightsCurve1;
	}

	float sumX = 0.0f;
	float sumY = 0.0f;
	float denominator = 0.0f;

	for (int k = 0; k <= degree; k++) {
		float bin = binomial(degree, k);
		denominator += weightControls[k] * bin * powf((1.0f - t), (float)(degree - k)) * powf(t, (float)k);
		sumX += weightControls[k] * controlPoints[k][X] * bin * powf((1.0f - t), (float)(degree - k)) * powf(t, (float)k);
		sumY += weightControls[k] * controlPoints[k][Y] * bin * powf((1.0f - t), (float)(degree - k)) * powf(t, (float)k);
	}
	result[X] = sumX/denominator;
	result[Y] = sumY/denominator;
}

void drawBezier(int curveToDraw) {
	vector<vector<float>> controlPoints;
	int segNum;
	if (curveToDraw == 0) {
		controlPoints = controlPointsCurve0;
		segNum = segNumC0;
	}
	else {
		controlPoints = controlPointsCurve1;
		segNum = segNumC1;
	}

	int degree = controlPoints.size()-1;
	if (segNum < 1 || degree < 1) {
		return;
	}
	float step = 1.0f / segNum;
	vector<vector<float>> vertices;
	for (float t = 0.0f; t <= 1.0f; t += step) {
		vector<float> result{ 0.0f, 0.0f };
		Bezier(degree, t, result, controlPoints, curveToDraw);
		vertices.push_back(result);
	}
	if (curveToDraw == 0) setColor(colorBezierC0);
	else setColor(colorBezierC1);
	glBegin(GL_LINES);
	for (unsigned int i = 0; i < vertices.size() - 1; i++) {
		glVertex2f(vertices[i][X], vertices[i][Y]);
		glVertex2f(vertices[i + 1][X], vertices[i + 1][Y]);
	}
	glEnd();
}

void drawExtendedBezier(int curveToDraw) {
	// Get correct parameters
	vector<vector<float>> controlPoints;
	int segNum;
	if (curveToDraw == 0) {
		controlPoints = controlPointsCurve0;
		segNum = segNumC0;
	}
	else {
		controlPoints = controlPointsCurve1;
		segNum = segNumC1;
	}

	int degree = controlPoints.size() - 1;
	if (segNum < 1 || degree < 1) {
		return;
	}
	float step = 1.0f / segNum;
	vector<vector<float>> vertices;
	// Iterate from t = -5 to 0
	for (float t = -5.0f; t <= 0.0f; t += step) {
		vector<float> result{ 0.0f, 0.0f };
		Bezier(degree, t, result, controlPoints, curveToDraw);
		vertices.push_back(result);
	}
	// Draw lower extended curve
	if (curveToDraw == 0) setColor(colorExtendedC0);
	else setColor(colorExtendedC1);
	glBegin(GL_LINES);
	for (unsigned int i = 0; i < vertices.size() - 1; i++) {
		glVertex2f(vertices[i][X], vertices[i][Y]);
		glVertex2f(vertices[i + 1][X], vertices[i + 1][Y]);
	}
	glEnd();
	vertices = vector<vector<float>>(0);
	// Iterate from t = 0 to t = 6
	for (float t = 0.0f; t <= 6.0f; t += step) {
		vector<float> result{ 0.0f, 0.0f };
		Bezier(degree, t, result, controlPoints, curveToDraw);
		vertices.push_back(result);
	}
	// Draw upper extended curve
	glBegin(GL_LINES);
	for (unsigned int i = 0; i < vertices.size() - 1; i++) {
		glVertex2f(vertices[i][X], vertices[i][Y]);
		glVertex2f(vertices[i + 1][X], vertices[i + 1][Y]);
	}
	glEnd();
}

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|				Weight functions					|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

void drawAxisGraphs() {
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex2f(-0.9f, -0.9f);
	glVertex2f(-0.9f, 0.9f);
	glVertex2f(-0.9f, -0.3f);
	glVertex2f(0.9f, -0.3f);
	for (float i = -0.9f; i < 1.0f; i += 1.8f/15) {
		glVertex2f(-0.9f, i);
		glVertex2f(-0.95f, i);
	}
	glEnd();

}

void drawBars() {
	vector<float> *weightControls;
	if (selectedCurve == 0) {
		weightControls = &weightsCurve0;
	}
	else {
		weightControls = &weightsCurve1;
	}
	
	int size = weightControls->size();
	if (size <= 0) return;
	float xLen;
	if (size < 10) xLen = 0.2f;
	else xLen = 1.8f / size;

	float x = -0.9f;
	for (int i = 0; i < size; i++) {
		float y = 0.12f * (*weightControls)[i] - 0.3f;

		glColor3f(0.9f, 0.85f, 0.45f);
		if (y < -0.3f) glColor3f(1, 0, 0);
		glBegin(GL_POLYGON);
		glVertex2f(x, -0.3f);
		glVertex2f(x, y);
		glVertex2f(x + xLen, y);
		glVertex2f(x + xLen, -0.3f);
		glEnd();

		glColor3f(0, 0, 0);
		glBegin(GL_LINE_LOOP);
		glVertex2f(x, -0.3f);
		glVertex2f(x, y);
		glVertex2f(x + xLen, y);
		glVertex2f(x + xLen, -0.3f);
		glEnd();

		x += xLen;
	}
}

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|				MAIN Callback functions				|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

void displayFunction() {
	
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFlush();

	drawGrid();

	if (drawHullC0) {
		drawConvexHull(0);
	}
	if (drawHullC1) {
		drawConvexHull(1);
	}
	if (drawControlPolyC0) {
		drawControlPolygon(0);
	}
	if (drawControlPolyC1) {
		drawControlPolygon(1);
	}
	if (drawExtendedCurveC0) {
		drawExtendedBezier(0);
	}
	if (drawExtendedCurveC1) {
		drawExtendedBezier(1);
	}
	if (radioSelectedModeC0 == 3) {
		drawDeCasteljau(controlPointsCurve0, weightsCurve0, deCastelTC0, 0);
	}
	if (radioSelectedModeC1 == 3) {
		drawDeCasteljau(controlPointsCurve1, weightsCurve1, deCastelTC1, 1);
	}
	if (drawCurveC0) {
		drawBezier(0);
	}
	if (drawCurveC1) {
		drawBezier(1);
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

void mouseFunction(int button, int state, int x, int y) {
	// If is not left button return
	if (button != GLUT_LEFT_BUTTON) return;
	if (state == GLUT_DOWN) {
		decreaseButton->disable();
		// Add new control point
		if (*radioSelectedMode == 0 && controlPointsCurve0.size() < maxNumberControls) {
			int screen[] = { x, y };
			vector<float> world{ 0.0f, 0.0f };
			screenToWorldCoords(screen, world);
			if (-1.01f > world[X] || world[X] > 1.01f
				|| -1.01f > world[Y] || world[Y] > 1.01f) {
				return;
			}
			if (selectedCurve == 0) {
				controlPointsCurve0.push_back(world);
				weightsCurve0.push_back(1.0f);
			}
			else {
				controlPointsCurve1.push_back(world);
				weightsCurve1.push_back(1.0f);
			}
			forceUpdateSec = 1;
		}
		// Delete point
		else if (*radioSelectedMode == 2 && controlPointsCurve0.size() > 0) {
			int screen[] = { x, y };
			vector<float> world{ 0.0f, 0.0f };
			screenToWorldCoords(screen, world);
			if (-1.01f > world[X] || world[X] > 1.01f
				|| -1.01f > world[Y] || world[Y] > 1.01f) {
				return;
			}
			if (selectedCurve == 0) {
				for (unsigned int i = 0; i < controlPointsCurve0.size(); i++) {
					if (world[X] - 0.02f <= controlPointsCurve0[i][X] && controlPointsCurve0[i][X] <= world[X] + 0.02f
						&& world[Y] - 0.02f <= controlPointsCurve0[i][Y] && controlPointsCurve0[i][Y] <= world[Y] + 0.02f) {
						controlPointsCurve0.erase(controlPointsCurve0.begin() + i);
						weightsCurve0.erase(weightsCurve0.begin() + i);
					}
				}
			}
			else {
				for (unsigned int i = 0; i < controlPointsCurve1.size(); i++) {
					if (world[X] - 0.02f <= controlPointsCurve1[i][X] && controlPointsCurve1[i][X] <= world[X] + 0.02f
						&& world[Y] - 0.02f <= controlPointsCurve1[i][Y] && controlPointsCurve1[i][Y] <= world[Y] + 0.02f) {
						controlPointsCurve1.erase(controlPointsCurve1.begin() + i);
						weightsCurve1.erase(weightsCurve1.begin() + i);
					}
				}
			}
			forceUpdateSec = 1;
		}
		// Move point
		else if (*radioSelectedMode == 1) {
			int screen[] = { x, y };
			vector<float> world{ 0.0f, 0.0f };
			screenToWorldCoords(screen, world);
			if (-1.01f > world[X] || world[X] > 1.01f
				|| -1.01f > world[Y] || world[Y] > 1.01f) {
				return;
			}
			selectedPoint = -1;
			if (selectedCurve == 0) {
				for (unsigned int i = 0; i < controlPointsCurve0.size(); i++) {
					if (world[X] - 0.02f <= controlPointsCurve0[i][X] && controlPointsCurve0[i][X] <= world[X] + 0.02f
						&& world[Y] - 0.02f <= controlPointsCurve0[i][Y] && controlPointsCurve0[i][Y] <= world[Y] + 0.02f) {
						movingPoint = 1;
						originalPointPosition[X] = controlPointsCurve0[i][X];
						originalPointPosition[Y] = controlPointsCurve0[i][Y];
						selectedPoint = i;
					}
				}
			}
			else {
				for (unsigned int i = 0; i < controlPointsCurve1.size(); i++) {
					if (world[X] - 0.02f <= controlPointsCurve1[i][X] && controlPointsCurve1[i][X] <= world[X] + 0.02f
						&& world[Y] - 0.02f <= controlPointsCurve1[i][Y] && controlPointsCurve1[i][Y] <= world[Y] + 0.02f) {
						movingPoint = 1;
						originalPointPosition[X] = controlPointsCurve1[i][X];
						originalPointPosition[Y] = controlPointsCurve1[i][Y];
						selectedPoint = i;
					}
				}
			}
		}
	}
	else if (state == GLUT_UP) {
		if (*radioSelectedMode == 1 && selectedPoint >= 0) {
			int screen[] = { x, y };
			vector<float> world{ 0.0f, 0.0f };
			screenToWorldCoords(screen, world);
			if (selectedCurve == 0) {
				if (-1.01f > world[X] || world[X] > 1.01f
					|| -1.01f > world[Y] || world[Y] > 1.01f) {
					controlPointsCurve0[selectedPoint][X] = originalPointPosition[X];
					controlPointsCurve0[selectedPoint][Y] = originalPointPosition[Y];
				}
				else {
					controlPointsCurve0[selectedPoint][X] = world[X];
					controlPointsCurve0[selectedPoint][Y] = world[Y];
				}
			}
			else {
				if (-1.01f > world[X] || world[X] > 1.01f
					|| -1.01f > world[Y] || world[Y] > 1.01f) {
					controlPointsCurve1[selectedPoint][X] = originalPointPosition[X];
					controlPointsCurve1[selectedPoint][Y] = originalPointPosition[Y];
				}
				else {
					controlPointsCurve1[selectedPoint][X] = world[X];
					controlPointsCurve1[selectedPoint][Y] = world[Y];
				}
			}
			movingPoint = 0;
			selectedPoint = -1;
		}
	}
}

void motionFunction(int x, int y) {
	// If moving point to determine t in deCasteljau
	if (*radioSelectedMode == 3) {
		// Set current curve and t
		vector<vector<float>> controlPoints;
		float *newDeCastelT;
		if (selectedCurve == 0) {
			controlPoints = controlPointsCurve0;
			newDeCastelT = &deCastelTC0;
		}
		else {
			controlPoints = controlPointsCurve1;
			newDeCastelT = &deCastelTC1;
		}
		if (controlPoints.size() < 2) return;
		// Get point clicked
		int screen[] = { x, y };
		vector<float> world{ 0.0f, 0.0f };
		screenToWorldCoords(screen, world);
		// Check if point is in line
		vector<float> a = controlPoints[0];
		vector<float> b = controlPoints[1];
		// Point is in line
		if (abs(distance(a, world) + distance(world, b) - distance(a, b)) < numeric_limits<float>::epsilon()+0.01f) {
			float totalLen = distance(a, b);
			float propT = distance(a, world);
			*newDeCastelT = propT / totalLen;
			*deCastelT = propT / totalLen;
			forceUpdateMain = 1;
			glui->sync_live();
		}
		else {
			return;
		}

	}
	// If moving point
	else if (movingPoint) {
		int screen[] = { x, y };
		vector<float> world{ 0.0f, 0.0f };
		screenToWorldCoords(screen, world);
		if (selectedCurve == 0) {
			controlPointsCurve0[selectedPoint][X] = world[X];
			controlPointsCurve0[selectedPoint][Y] = world[Y];
		}
		else {
			controlPointsCurve1[selectedPoint][X] = world[X];
			controlPointsCurve1[selectedPoint][Y] = world[Y];
		}
		forceUpdateMain = 1;
	}
}

// Idle callback is enabled for both MAIN and sec
void idleFunction() {
	if (!forceUpdateSec && !forceUpdateMain) {
		return;
	}
	// Secondary window idle only when requested
	if (forceUpdateSec) {
		if (glutGetWindow() != secWindowID) glutSetWindow(secWindowID);
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFlush();

		drawAxisGraphs();
		drawBars();

		forceUpdateSec = 0;

		glFlush();
	}

	// Main window idle only when requested
	if (forceUpdateMain) {
		if (glutGetWindow() != mainWindowID) glutSetWindow(mainWindowID);

		glClearColor(GetRValue(colorBackground) / 255.0f, GetGValue(colorBackground) / 255.0f, GetBValue(colorBackground) / 255.0f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFlush();

		drawGrid();

		if (drawHullC0) {
			drawConvexHull(0);
		}
		if (drawHullC1) {
			drawConvexHull(1);
		}
		if (drawControlPolyC0) {
			drawControlPolygon(0);
		}
		if (drawControlPolyC1) {
			drawControlPolygon(1);
		}
		if (drawExtendedCurveC0) {
			drawExtendedBezier(0);
		}
		if (drawExtendedCurveC1) {
			drawExtendedBezier(1);
		}
		if (radioSelectedModeC0 == 3) {
			drawDeCasteljau(controlPointsCurve0, weightsCurve0, deCastelTC0, 0);
		}
		if (radioSelectedModeC1 == 3) {
			drawDeCasteljau(controlPointsCurve1, weightsCurve1, deCastelTC1, 1);
		}
		if (drawCurveC0) {
			drawBezier(0);
		}
		if (drawCurveC1) {
			drawBezier(1);
		}
		drawPoints();

		forceUpdateMain = 0;
		glFlush();
	}
}

void menuCallback(int choice) {
	switch (choice) {
	case 0:
		colorBackground = getColor(colorBackground);
		break;
	case 1:
		colorGrid = getColor(colorGrid);
		break;
	case 20:
		colorControlPointC0 = getColor(colorControlPointC0);
		break;
	case 21:
		colorControlPointHoveringC0 = getColor(colorControlPointHoveringC0);
		break;
	case 22:
		colorControlPolyC0 = getColor(colorControlPolyC0);
		break;
	case 23:
		colorDeCastelLinesC0 = getColor(colorDeCastelLinesC0);
		break;
	case 24:
		colorDeCastelPointsC0 = getColor(colorDeCastelPointsC0);
		break;
	case 25:
		colorDeCastelMainC0 = getColor(colorDeCastelMainC0);
		break;
	case 26:
		colorConvexC0 = getColor(colorConvexC0);
		break;
	case 27:
		colorBezierC0 = getColor(colorBezierC0);
		break;
	case 28:
		colorExtendedC0 = getColor(colorExtendedC0);
		break;
	case 30:
		colorControlPointC1 = getColor(colorControlPointC1);
		break;
	case 31:
		colorControlPointHoveringC1 = getColor(colorControlPointHoveringC1);
		break;
	case 32:
		colorControlPolyC1 = getColor(colorControlPolyC1);
		break;
	case 33:
		colorDeCastelLinesC1 = getColor(colorDeCastelLinesC1);
		break;
	case 34:
		colorDeCastelPointsC1 = getColor(colorDeCastelPointsC1);
		break;
	case 35:
		colorDeCastelMainC1 = getColor(colorDeCastelMainC1);
		break;
	case 36:
		colorConvexC1 = getColor(colorConvexC1);
		break;
	case 37:
		colorBezierC1 = getColor(colorBezierC1);
		break;
	case 38:
		colorExtendedC1 = getColor(colorExtendedC1);
		break;
	default:
		break;
	}
	forceUpdateMain = 1;

}

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|				Sec Callback functions				|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

void secondaryMouseFunction(int button, int state, int cx, int cy) {
	// If is not left button return
	if (button != GLUT_LEFT_BUTTON) return;
	if (state == GLUT_DOWN) {
		int screen[] = { cx, cy };
		vector<float> world{ 0.0f, 0.0f };
		screenToWorldCoordsSecondary(screen, world);

		if (-1.01f > world[X] || world[X] > 1.01f
			|| -1.01f > world[Y] || world[Y] > 1.01f) {
			return;
		}

		vector<float> *weightControls;
		if (selectedCurve == 0) {
			weightControls = &weightsCurve0;
		}
		else {
			weightControls = &weightsCurve1;
		}

		int size = weightControls->size();
		if (size <= 0) return;
		float xLen;
		if (size < 10) xLen = 0.2f;
		else xLen = 1.8f / size;

		selectedWeightModify = -1;

		float x = -0.9f;
		for (int i = 0; i < size; i++) {
			float y = 0.12f * (*weightControls)[i] - 0.3f;
			if (y - 0.02f <= world[Y] && world[Y] <= y + 0.02f
				&& x - 0.02f <= world[X] && world[X] <= x + xLen + 0.02f) {
				selectedWeightModify = i;
				movingBar = 1;
				break;
			}
			x += xLen;
		}
	}
	else if(state == GLUT_UP) {
		int screen[] = { cx, cy };
		vector<float> world{ 0.0f, 0.0f };
		screenToWorldCoordsSecondary(screen, world);
		if (-1.01f > world[X] || world[X] > 1.01f
			|| -1.01f > world[Y] || world[Y] > 1.01f
			|| world[Y] > 0.9f || world[Y] < -0.9f
			|| selectedWeightModify == -1) {
			selectedWeightModify = -1;
			selectedHoverBar = -1;
			forceUpdateMain = 1;
			return;
		}

		vector<float> *weightControls;
		if (selectedCurve == 0) {
			weightControls = &weightsCurve0;
		}
		else {
			weightControls = &weightsCurve1;
		}

		float newWeight = (world[Y] + 0.3f) / 0.12f;
		(*weightControls)[selectedWeightModify] = newWeight;
		movingBar = 0;
		selectedWeightModify = -1;
		selectedHoverBar = -1;
		forceUpdateMain = 1;
	}
}

void secondaryPassiveMotionFunction(int cx, int cy) {
	int screen[] = { cx, cy };
	vector<float> world{ 0.0f, 0.0f };
	screenToWorldCoordsSecondary(screen, world);
	
	if (-1.01f > world[X] || world[X] > 1.01f
		|| -1.01f > world[Y] || world[Y] > 1.01f
		|| world[Y] > 0.9f || world[Y] < -0.9f) {
		if (selectedHoverBar != -1 || selectedWeightModify != -1) forceUpdateMain = 1;
		selectedWeightModify = -1;
		selectedHoverBar = -1;
		return;
	}

	vector<float> *weightControls;
	if (selectedCurve == 0) {
		weightControls = &weightsCurve0;
	}
	else {
		weightControls = &weightsCurve1;
	}
	selectedWeightModify = -1;
	selectedHoverBar = -1;

	int size = weightControls->size();
	if (size <= 0) return;
	float xLen;
	if (size < 10) xLen = 0.2f;
	else xLen = 1.8f / size;

	float x = -0.9f;
	for (int i = 0; i < size; i++) {
		float y = 0.12f * (*weightControls)[i] - 0.3f;
		if ((y > -0.3f && (-0.3f - 0.01f <= world[Y] && world[Y] <= y + 0.01f
			&& x - 0.01f <= world[X] && world[X] <= x + xLen + 0.01f))
			|| (y < -0.3f && (y - 0.01f <= world[Y] && world[Y] <= -0.3f + 0.01f
			&& x - 0.01f <= world[X] && world[X] <= x + xLen + 0.01f))) {
			selectedHoverBar = i;
			break;
		}
		x += xLen;
	}
	forceUpdateMain = 1;
}

void secondaryMotionFunction(int cx, int cy) {
	if (movingBar) {
		int screen[] = { cx, cy };
		vector<float> world{ 0.0f, 0.0f };
		screenToWorldCoordsSecondary(screen, world);
		if (-1.01f > world[X] || world[X] > 1.01f
			|| -1.01f > world[Y] || world[Y] > 1.01f
			|| world[Y] > 0.9f || world[Y] < -0.9f
			|| selectedWeightModify == -1) {
			return;
		}

		vector<float> *weightControls;
		if (selectedCurve == 0) {
			weightControls = &weightsCurve0;
		}
		else {
			weightControls = &weightsCurve1;
		}

		float newWeight = (world[Y] + 0.3f) / 0.12f;
		(*weightControls)[selectedWeightModify] = newWeight;

		forceUpdateSec = 1;
		forceUpdateMain = 1;
	}
}

void secondaryDisplayFunction() {
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFlush();

	drawAxisGraphs();
	drawBars();

	glFlush();
}

void secondaryResizeFunction(int width, int height) {
	if (width == 0 || height == 0) return;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	int deltaH = 0, deltaW = 0, ed = 0;
	if (width > height) ed = height;
	else ed = width;
	deltaH = abs(height - ed) / 2;
	deltaW = abs(width - ed) / 2;
	glViewport(deltaW, deltaH, ed, ed);
}

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|					   Main							|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

void buildMenus() {
	int menuC0 = glutCreateMenu(menuCallback);
	glutAddMenuEntry("Control point color", 20);
	glutAddMenuEntry("Control point (Hovering) color", 21);
	glutAddMenuEntry("Control polynomial color", 22);
	glutAddMenuEntry("deCasteljau (Lines) color", 23);
	glutAddMenuEntry("deCasteljau (Points) color", 24);
	glutAddMenuEntry("deCasteljau (Point on curve) color", 25);
	glutAddMenuEntry("Convex hull color", 26);
	glutAddMenuEntry("Bezier curve color", 27);
	glutAddMenuEntry("Bezier extended curve color", 28);

	int menuC1 = glutCreateMenu(menuCallback);
	glutAddMenuEntry("Control point color", 30);
	glutAddMenuEntry("Control point (Hovering) color", 31);
	glutAddMenuEntry("Control polynomial color", 32);
	glutAddMenuEntry("deCasteljau (Lines) color", 33);
	glutAddMenuEntry("deCasteljau (Points) color", 34);
	glutAddMenuEntry("deCasteljau (Point on curve) color", 35);
	glutAddMenuEntry("Convex hull color", 36);
	glutAddMenuEntry("Bezier curve color", 37);
	glutAddMenuEntry("Bezier extended curve color", 38);

	glutCreateMenu(menuCallback);
	glutAddMenuEntry("Background color", 0);
	glutAddMenuEntry("Grid color", 1);
	glutAddSubMenu("Curve 1 colors", menuC0);
	glutAddSubMenu("Curve 2 colors", menuC1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char** argv) {

	// Basic OpenGL initial setup
	glutInit(&argc, argv);

	// ~~~~~~~~~~ MAIN WINDOW INIT ~~~~~~~~~~
	glutInitWindowSize(mainEdge, mainEdge);
	glutInitWindowPosition(300, 300);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	mainWindowID = glutCreateWindow("Montoya_Miguel A5_2 MAIN");
	glutSetWindow(mainWindowID);
	// Set up
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1.01, 1.01, -1.01, 1.01);
	glClearColor(GetRValue(colorBackground) / 255.0f, GetGValue(colorBackground) / 255.0f, GetBValue(colorBackground) / 255.0f, 1);
	buildMenus();
	// Set up callbacks (All done by GLUI, except motion)
	glutMotionFunc(motionFunction);
	// Create GUI
	buildGLUI(mainWindowID);

	// ~~~~~~~~~~ SECOND WINDOW INIT ~~~~~~~~~~
	glutInitWindowSize(secEdge, secEdge);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	secWindowID = glutCreateWindow("Montoya_Miguel A5_2");

	// Set up
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1.01, 1.01, -1.01, 1.01);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	// Set up callbacks
	glutDisplayFunc(secondaryDisplayFunction);
	glutReshapeFunc(secondaryResizeFunction);
	glutIdleFunc(idleFunction);
	glutMotionFunc(secondaryMotionFunction);
	glutPassiveMotionFunc(secondaryPassiveMotionFunction);
	glutMouseFunc(secondaryMouseFunction);
	
	// Enter main event loop
	glutMainLoop();
	return 0;
}