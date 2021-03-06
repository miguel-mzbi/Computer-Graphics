#pragma once

// Sample Polynomials
const char *samples[12] = {
	"x^2+y^2-0.8",							// Circle
	"x^2+4*y^2-0.5",						// Ellipse
	"x^2-4*y^2-0.09",						// Hyperbola
	"y^2-x^3",								// Semicubic parabola
	"y^2-x^2-x^3",							// Alpha curve
	"y^2-x^4-y^4",							// Touching Tacnode
	"y^2-x^6-y^6",							// Crossing Tacnode
	"(x^2+y^2)^2-(x^2-y^2)",				// Lemniscate
	"8*x^4-6*1.1*x^2*y+1.21*y^2-4*1.331*y^3+4*1.21*1.21*y^4",	// Rabbit Ears
	"x^4+y^2*(x^2-x+1)*1.44-2*y*x^2*1.2",	// Curved Love
	"(x^2+y^2)^2+3*x^2*y-y^3",				// Trefoil
	"(x^2+y^2)^3-4*x^2*y^2",				// Clover Leaf
};

static void updateAndClean();
static void clear();
static void drawAxis();
static void refresh();

static void buildEvalMatrix();
static void deleteEvalMatrix(int);

static void getVertexA(float x, float y, std::vector<float> &);
static bool differentVertex(std::vector<float> &);
static void drawBoxLines(std::vector<float> &);
static void drawLetter(float, float);
static void findIntersection();
static void findBoxes(std::vector<float> &);

static void getPointsAll();
static void drawPointsAll();
static float getLineCoordBox(float, float);
static void drawLinesAll();