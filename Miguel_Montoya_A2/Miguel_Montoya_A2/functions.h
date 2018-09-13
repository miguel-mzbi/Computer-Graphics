#pragma once

static void draw();
static void clearBothBuffers();
static void refresh();
static void displayTea();
static void screenToPlaneCoords(int, int, std::vector<float> &);
static float computeAngle();