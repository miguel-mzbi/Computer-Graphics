#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include <vector>
#include <iostream>
#include "GL/freeglut.h"
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
int initialWidth = 200;
int initialHeight = 200;
float m = 0; // 2m + 1 = pixels in one side

// Scene
Color const bgColor = Color(0.0, 0.5, 1.0, 1.0);
Buffer buffer;

// Light
vector<float> lightPos{1, 2, -1};
Light light0 = Light(lightPos,
					Color(0.2f, 0.2f, 0.2f, 1.0f),
					Color(1.0f, 1.0f, 1.0f, 1.0f),
					Color(1.0f, 1.0f, 1.0f, 1.0f));

// Camera
float e = 1; // Distance from plane to camera
float fov = (float)M_PI / 6.0f;
int cameraDirection = -1; // Towards negative z
vector<float> cameraPos(4);

// Objects
float c = 2; // Distance from center of sphere to plane
float r = 1; // Default radius of sphere
Material jadeMaterial = Material(12.8f, 
							Color(0.135f, 0.2225f, 0.1575f, 0.95f), 
							Color(0.54f, 0.89f, 0.63f, 0.95f), 
							Color(0.316228f, 0.316228f, 0.316228f, 0.95f));
vector<float> sphere1Pos;
Sphere sphere1;

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|			OpenGl Utility functions				|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

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
		// If it was a sphere (Object = 1)
		if (correspondingObject == 1) {
			findSphereNormal(sphere1, p, normal);
			material = sphere1.material;
		}
	}
	return hit;
}

void getShade(vector<float> &intersection, vector<float> &normal, Material &material, Color &color) {
	float r = material.ambientC.r * light0.ambientC.r;
	float g = material.ambientC.g * light0.ambientC.g;
	float b = material.ambientC.b * light0.ambientC.b;
	float a = material.ambientC.a * light0.ambientC.a;
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
	
	// Start at left column, increment column at each outside for iteration
	currentWorldP[X] = (0 - (initialWidth / 2.0f)) * (1.0f / (initialWidth / 2.0f)) + (1.0f / m) / 2.0f;
	for (int i = 0; i < initialWidth; i++) {
		// Start at bottom row, increment row at each inner for iteration
		currentWorldP[Y] = (0 - (initialHeight / 2.0f)) * (1.0f / (initialHeight / 2.0f)) + (1.0f / m) / 2.0f;
		for (int j = 0; j < initialHeight; j++) {
			Color pixelColor;
			cout << "x:" << currentWorldP[X] << " | " << "y:" << currentWorldP[Y] << "\n";
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
	glClear(GL_COLOR_BUFFER_BIT);

	calculateScene();
	buffer.drawBuffer();

	glFlush();
}

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

	buffer = Buffer(initialWidth, initialHeight);

	initCamera();
	createObjects();
}

// Create camera
void initCamera() {
	cameraPos = vector<float>({ 0.0, 0.0, e, 1 });
}

// Create virtual objects in world
void createObjects() {
	vector<float> pos{ 0.0, 0.0, -c };
	sphere1Pos = pos;
	sphere1 = Sphere(r, sphere1Pos, jadeMaterial);
}

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|					   Main							|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

int main(int argc, char** argv) {
	// Compute m
	m = (initialWidth - 1) / 2.0f;

	// Basic OpenGL initial setup
	glutInit(&argc, argv);
	glutInitWindowSize(initialWidth, initialHeight);
	glutInitWindowPosition(350, 350);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	windowID = glutCreateWindow("Montoya_Miguel A3");
	glutSetWindow(windowID);
	
	// Initialize basics
	initWorld();

	glutDisplayFunc(displayFunction);
	glutMainLoop();
	return 0;
}