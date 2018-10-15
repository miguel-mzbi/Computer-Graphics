#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include <vector>
#include <iostream>
#include <algorithm>
#include "GL/glui.h"
#include "GL/freeglut.h"
#include "BuildGLUI.h"
#include "Color.h"
#include "Light.h"
#include "Material.h"
#include "Sphere.h"
#include "Buffer.h"
#include "RayTrace.h"

using namespace std;

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|					Parameters						|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

// Substitutions
int START = 0;
int END = 1;
int X = 0;
int Y = 1;
int Z = 2;
int W = 3;

// Basics 
int windowID;
int edge = 101;
float m = 0; // 2m + 1 = pixels in one side
int shadowActive = 0;

// Scene
Color const bgColor = Color(0.0, 0.5, 1.0, 1.0);
Buffer buffer;

// Light
vector<float> lightPos{ 1, 2, -1, 0 };
Color ambientLight = Color(0.2f, 0.2f, 0.2f, 1.0f);
Color diffuseLight = Color(1.0f, 1.0f, 1.0f, 1.0f);
Color specularLight = Color(1.0f, 1.0f, 1.0f, 1.0f);
Light light0;

// Camera
float e = 1; // Distance from plane to camera
float fov = (float)M_PI / 6.0f;
int cameraDirection = -1; // Towards negative z
vector<float> cameraPos(4);

// Materials
Material jadeMaterial = Material(12.8f,
	Color(0.135f, 0.2225f, 0.1575f, 0.95f),
	Color(0.54f, 0.89f, 0.63f, 0.95f),
	Color(0.316228f, 0.316228f, 0.316228f, 0.95f));

Material blackRubberMaterial = Material(10.0f,
	Color(0.02f, 0.02f, 0.02f, 1.0f),
	Color(0.01f, 0.01f, 0.01f, 1.0f),
	Color(0.4f, 0.4f, 0.4f, 1.0f));

Material whiteRubberMaterial = Material(10.0f,
	Color(0.05f, 0.05f, 0.05f, 1.0f),
	Color(0.5f, 0.5f, 0.5f, 1.0f),
	Color(0.7f, 0.7f, 0.7f, 1.0f));

// Objects
float r = 1; // Default radius of sphere / Chess board located at -r
// Sphere
float c = 2; // Distance from center of sphere to plane
vector<float> sphere1Pos;
Sphere sphere1;
// Plane
vector<float> v1(4); // 3 points defining the plane
vector<float> v2(4);
vector<float> v3(4);
vector<float> planeNormal; // Normal to board

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|				Function declarations				|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

void calculateDirection(vector<float> &startP, vector<float> &finalP, vector<float> &direction);
void initWorld();
void initCamera();
void initLight();
void createObjects();

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|				Utility functions					|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

void dotProduct(vector<float> &A, vector<float> &B, float &result) {
	result = A[X] * B[X] + A[Y] * B[Y] + A[Z] * B[Z];
}

void scalarProduct(vector<float> &A, float &scalar, vector<float> &result) {
	result[X] = A[X] * scalar;
	result[Y] = A[Y] * scalar;
	result[Z] = A[Z] * scalar;
}

void vectorSubstraction(vector<float> &A, vector<float> &B, vector<float> &result) {
	result[X] = A[X] - B[X];
	result[Y] = A[Y] - B[Y];
	result[Z] = A[Z] - B[Z];
}

void normalize(vector<float> &A) {
	float c = sqrtf(A[X] * A[X] + A[Y] * A[Y] + A[Z] * A[Z]);
	A[X] = A[X] / c;
	A[Y] = A[Y] / c;
	A[Z] = A[Z] / c;
}

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|				Ray Tracing functions				|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

void findSphereNormal(Sphere &sphere, vector<float> &p, vector<float> &normal) {
	normal[X] = (p[X] - sphere.center[X]) / sphere.r;
	normal[Y] = (p[Y] - sphere.center[Y]) / sphere.r;
	normal[Z] = (p[Z] - sphere.center[Z]) / sphere.r;
	normal[W] = 0.0;
}

void findCoord(vector<vector<float>*> &ray, float t, vector<float> &p) {
	vector<float> start(4);
	vector<float> end(4);
	start = *ray[START];
	end = *ray[END];
	p[X] = start[X] + t * end[X];
	p[Y] = start[Y] + t * end[Y];
	p[Z] = start[Z] + t * end[Z];
	p[W] = 1.0f;
}

int sphereIntersection(vector<vector<float>*> &ray, Sphere &sphere, float &t) {
	vector<float> tStart(4);
	vector<float> tEnd(4);
	vector<float> start(4);
	vector<float> end(4);
	start = *ray[START];
	end = *ray[END];
	// Translate sphere to origin (More easy to calculate intersection)
	tStart[X] = start[X] - sphere.center[X];
	tStart[Y] = start[Y] - sphere.center[Y];
	tStart[Z] = start[Z] - sphere.center[Z];
	tEnd = end;

	// Start intersection operation
	// Get coefficients for quadratic
	float a = tEnd[X] * tEnd[X] + tEnd[Y] * tEnd[Y] + tEnd[Z] * tEnd[Z];
	float b = 2 * (tEnd[X] * tStart[X] + tEnd[Y] * tStart[Y] + tEnd[Z] * tStart[Z]);
	float c = tStart[X] * tStart[X] + tStart[Y] * tStart[Y] + tStart[Z] * tStart[Z] - sphere.r * sphere.r;

	// Discriminant
	float D = b * b - 4 * a*c;
	// Solution has complex numbers
	if (D < 0) {
		return 0;
	}
	else {
		D = sqrtf(D);
		// Try closest intersection (Lower t)
		t = (-b - D) / (2 * a);
		// If closest intersection (Lower t) is behind camera, try next t
		if (t < 0) {
			t = (-b + D) / (2 * a);
		}
		// If next t is behind camera too, no intersection was visible from our camera
		if (t < 0) {
			return 0;
		}
		else {
			return 1;
		}
	}
}

int planeIntersection(vector<vector<float>*> &ray, vector<float> &normal, vector<float> &point, float &t) {
	vector<float> start(4);
	vector<float> end(4);
	start = *ray[START];
	end = *ray[END];
	float den = normal[X] * end[X] + normal[Y] * end[Y] + normal[Z] * end[Z];
	// Return no hit if there is no intersection (parallel)
	if (den == 0) return 0;
	float num = -normal[X] * start[X] - normal[Y] * start[Y] - normal[Z] * start[Z] + normal[X] * point[X] + normal[Y] * point[Y] + normal[Z] * point[Z];
	t = num / den;
	// Return no hit if intersection is behind
	if (t < 0) return 0;
	else return 1;

}

int trace(vector<vector<float>*> &ray, vector<float> &p, vector<float> &normal, Material &material) {
	vector<float> intersections;
	vector<int> objects;
	int hit = 0;
	float t = 0;
	if (sphereIntersection(ray, sphere1, t)) {
		hit = 1;
		intersections.push_back(t);
		objects.push_back(1);
	}
	if (planeIntersection(ray, planeNormal, v1, t)) {
		hit = 1;
		intersections.push_back(t);
		objects.push_back(0);
	}
	// Get closest t from all objects
	if (hit) {
		float smallestT = numeric_limits<float>::max();
		int correspondingObject = -1;
		for (unsigned int i = 0; i < intersections.size(); i++) {
			if (intersections[i] < smallestT) {
				smallestT = intersections[i];
				correspondingObject = objects[i];
			}
		}
		// Find coordinate of intersection with closest
		findCoord(ray, smallestT, p);
		if (correspondingObject == 0) {
			normal = planeNormal;
			float modulX;
			float modulZ;
			if (p[X] < 0) {
				modulX = fmodf(fmodf(p[X], 2.0f) + 2.0f, 2.0f);
			}
			else {
				modulX = fmodf(p[X], 2.0f);
			}
			if (p[Z] < 0) {
				modulZ = fmodf(fmodf(p[Z], 2.0f) + 2.0f, 2.0f);
			}
			else {
				modulZ = fmodf(p[Z], 2.0f);
			}

			if (modulZ < 1) {
				if (modulX < 1) {
					material = blackRubberMaterial;
				}
				else {
					material = whiteRubberMaterial;
				}
			}
			else {
				if (modulX < 1) {
					material = whiteRubberMaterial;
				}
				else {
					material = blackRubberMaterial;
				}
			}
			
		}
		// If it was a sphere (Object = 1)
		else if (correspondingObject == 1) {
			findSphereNormal(sphere1, p, normal);
			material = sphere1.material;
		}
	}
	return hit;
}

void getShade(vector<float> &intersection, vector<float> &normal, Material &material, Color &color) {
	normalize(normal);
	vector<float> lightVector(4);
	calculateDirection(intersection, lightPos, lightVector);
	//calculateDirection(lightPos, intersection, lightVector);
	normalize(lightVector);

	vector<float> cameraVector(4);
	calculateDirection(intersection, cameraPos, cameraVector);
	//calculateDirection(lightPos, intersection, cameraVectorR);
	normalize(cameraVector);

	float diffuseValue;
	dotProduct(lightVector, normal, diffuseValue);
	diffuseValue = max(0.0f, diffuseValue);

	float specularValue;

	float dot;
	dotProduct(normal, lightVector, dot);
	float scalar = 2.0f * dot;
	vector<float> nScalar(4);
	scalarProduct(normal, scalar, nScalar);
	vector<float> rVector(4);
	vectorSubstraction(nScalar, lightVector, rVector);
	dotProduct(cameraVector, rVector, specularValue);
	specularValue = max(0.0f, specularValue);
	float specularExp = powf(specularValue, material.shine);


	float r = material.ambientC.r * light0.ambientC.r 
			+ material.diffuseC.r * light0.diffuseC.r * diffuseValue 
			+ material.specularC.r * light0.specularC.r * specularExp;
	float g = material.ambientC.g * light0.ambientC.g
			+ material.diffuseC.g * light0.diffuseC.g * diffuseValue 
			+ material.specularC.g * light0.specularC.g * specularExp;
	float b = material.ambientC.b * light0.ambientC.b 
			+ material.diffuseC.b * light0.diffuseC.b * diffuseValue 
			+ material.specularC.b * light0.specularC.b * specularExp;
	float a = material.ambientC.a * light0.ambientC.a 
			+ material.diffuseC.a * light0.diffuseC.a * diffuseValue 
			+ material.specularC.a * light0.specularC.a * specularExp;

	if (r > 1.0) r = 1.0;
	if (g > 1.0) g = 1.0;
	if (b > 1.0) b = 1.0;
	if (a > 1.0) a = 1.0;

	if (r < 0.0) r = 0.0;
	if (g < 0.0) g = 0.0;
	if (b < 0.0) b = 0.0;
	if (a < 0.0) a = 0.0;

	color = Color(r, g, b, a);
}

void findColor(vector<vector<float>*> &ray, Color &color) {
	vector<float> intersection(4);
	vector<float> normal(4);
	Material intersectionMaterial;
	if (trace(ray, intersection, normal, intersectionMaterial)) {
		getShade(intersection, normal, intersectionMaterial, color);
	}
	else {
		color = bgColor;
	}
}

void calculateDirection(vector<float> &startP, vector<float> &finalP, vector<float> &direction) {
	direction[X] = finalP[X] - startP[X];
	direction[Y] = finalP[Y] - startP[Y];
	direction[Z] = finalP[Z] - startP[Z];
	direction[W] = 0;
}

void calculateScene() {

	// Current point in plane
	vector<float> currentWorldP(4);
	// Constant values in all plane only x and y change
	currentWorldP[Z] = 0; // z = 0
	currentWorldP[W] = 1; // w = 1
	// Direction of ray, w = 0, because ray ends at infinity
	vector<float> direction(4);
	// Ray with start at WordlP ending at the infinity
	vector<vector<float>*> ray(2);
	ray[START] = &currentWorldP;
	ray[END] = &direction;
	int bv = 0;
	
	// Start at left column, increment column at each outside for iteration
	currentWorldP[X] = (0 - (edge / 2.0f)) * (1.0f / (edge / 2.0f)) + (1.0f / m) / 2.0f;
	for (int i = 0; i < edge; i++) {
		//cout << "x:" << currentWorldP[X] << "\n";
		// Start at bottom row, increment row at each inner for iteration
		currentWorldP[Y] = (0 - (edge / 2.0f)) * (1.0f / (edge / 2.0f)) + (1.0f / m) / 2.0f;
		
		for (int j = 0; j < edge; j++) {
			Color pixelColor;
			// Direction towards position in plane
			calculateDirection(cameraPos, currentWorldP, direction);
			// Find color of pixel
			findColor(ray, pixelColor);
			buffer.drawPixel(i, j, pixelColor);
			currentWorldP[Y] += 1.0f / m; // Increment y by 1/m
		}
		currentWorldP[X] += 1.0f / m; // Increment x by 1/m
	}
}

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|				Callback functions					|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

void displayFunction() {
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFlush();
	
	// Basic setup for changes made via menu
	m = (edge - 1) / 2.0f;
	buffer = Buffer(edge, edge);
	initCamera();
	initLight();
	createObjects();

	calculateScene();
	buffer.drawBuffer();

	glFlush();
}

void resizeFunction(int width, int height) {
	if (width == 0 || height == 0) return;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	int deltaH = 0, deltaW = 0;
	deltaH = abs(height - edge) / 2;
	deltaW = abs(width - edge) / 2;
	glMatrixMode(GL_MODELVIEW);
	glViewport(deltaW, deltaH, edge, edge);
}

void mouseFunction(int button, int state, int x, int y) { ; }
void idleFunction() { ; }

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|					   Setup						|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

// Start world and buffer
void initWorld() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, 10.0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

void initLight() {
	light0 = Light(lightPos,
		ambientLight,
		diffuseLight,
		specularLight);		
}
// Create camera
void initCamera() {
	cameraPos = vector<float>({ 0.0, 0.0, e, 1 });
}

// Create virtual objects in world
void createObjects() {
	// Sphere
	vector<float> pos{ 0.0, 0.0, -c };
	sphere1Pos = pos;
	sphere1 = Sphere(r, sphere1Pos, jadeMaterial);

	// Board
	v1 = vector<float>({ 0.0f, -r, 0.0f, 1.0f });
	v2 = vector<float>({ 0.0f, -r, 1.0f, 1.0f });
	v3 = vector<float>({ 1.0f, -r, 0.0f, 1.0f });
	vector<float> a{ v2[X] - v1[X], v2[Y] - v1[Y], v2[Z] - v1[Z] };
	vector<float> b{ v3[X] - v1[X], v3[Y] - v1[Y], v3[Z] - v1[Z] };
	planeNormal = vector<float>({ a[Y] * b[Z] - a[Z] * b[Y], -1 * (a[X] * b[Z] - a[Z] * b[X]), a[X] * b[Y] - a[Y] * b[X] });
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
	glutInitWindowPosition(350, 350);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	windowID = glutCreateWindow("Montoya_Miguel A3");
	glutSetWindow(windowID);

	buildGLUI(windowID);
	
	// Initialize basics
	initWorld();

	// Set up callbacks
	glutDisplayFunc(displayFunction);
	glutReshapeFunc(resizeFunction);

	// Start loop
	glutMainLoop();
	return 0;
}