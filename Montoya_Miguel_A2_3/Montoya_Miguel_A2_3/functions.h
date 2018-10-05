#pragma once

static void draw();
static void clearBothBuffers();
static void refresh();
static void displayTea();
static void screenToPlaneCoords(int, int, std::vector<float> &);
static float computeAngleLocal();
static float computeAngleGlobal();
static void openDialog();
static void parsePoly(char*);
static COLORREF getColor();

static void initMenus();
static void initLights();