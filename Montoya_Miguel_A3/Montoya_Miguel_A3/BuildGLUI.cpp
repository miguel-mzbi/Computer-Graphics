#include <iostream>
#include <vector>
#include <GL/glui.h>
//#include "GL/freeglut.h"
#include "BuildGLUI.h"
#include "RayTrace.h"
#include "Color.h"

GLUI *glui;
GLUI *glui_left;
GLUI *glui_bottom;
GLUI_Checkbox *checkbox;
GLUI_Spinner *spinner;
GLUI_RadioGroup *radio;
GLUI_Panel *panel;
GLUI_EditText *edittext;
extern GLUI *glui;

extern int shadowActive;
extern float c;
extern float m;
extern int edge;
extern float e;
extern float r;
extern int lightAtInfinity;
extern std::vector<float> lightPos;
extern Color ambientLight;
extern Color diffuseLight;
extern Color specularLight;
int l = edge;

void voidRecall(int c) { ; }

void getM(int c) {
	edge = l;
	m = (edge - 1) / 2.0f;
	glutReshapeWindow(edge, edge);
}

void buildGLUI(int windowID) {

	glui = GLUI_Master.create_glui("GLUI", 0, 500, 500);
	glui->set_main_gfx_window(windowID);
	GLUI_Master.set_glutReshapeFunc(resizeFunction);

	glui->add_statictext("Move GLUI window if spawned");
	glui->add_statictext("above OpenGl window.");

	GLUI_Panel *panel1 = new GLUI_Panel(glui, "Shadow", GLUI_PANEL_EMBOSSED);
	new GLUI_Checkbox(panel1, "Shadow Active", &shadowActive, 11, voidRecall);

	GLUI_Panel *panel2 = new GLUI_Panel(glui, "Sphere", GLUI_PANEL_EMBOSSED);
	glui->add_statictext_to_panel(panel2, "Distance of sphere to plane.");
	new GLUI_EditText(panel2, "c (float)", &c, 21, voidRecall);
	glui->add_statictext_to_panel(panel2, "Radius of sphere.");
	new GLUI_EditText(panel2, "r (float)", &r, 22, voidRecall);

	glui->add_column(true);

	GLUI_Panel *panel3 = new GLUI_Panel(glui, "Light", GLUI_PANEL_EMBOSSED);
	glui->add_statictext_to_panel(panel3, "Ambient component.");
	new GLUI_EditText(panel3, "r (float)", &ambientLight.r, 311, voidRecall);
	new GLUI_EditText(panel3, "g (float)", &ambientLight.g, 312, voidRecall);
	new GLUI_EditText(panel3, "b (float)", &ambientLight.b, 313, voidRecall);
	new GLUI_EditText(panel3, "a (float)", &ambientLight.a, 314, voidRecall);
	glui->add_statictext_to_panel(panel3, "Diffuse component.");
	new GLUI_EditText(panel3, "r (float)", &diffuseLight.r, 321, voidRecall);
	new GLUI_EditText(panel3, "g (float)", &diffuseLight.g, 322, voidRecall);
	new GLUI_EditText(panel3, "b (float)", &diffuseLight.b, 323, voidRecall);
	new GLUI_EditText(panel3, "a (float)", &diffuseLight.a, 324, voidRecall);
	glui->add_column_to_panel(panel3, true);
	glui->add_statictext_to_panel(panel3, "Specular component.");
	new GLUI_EditText(panel3, "r (float)", &specularLight.r, 331, voidRecall);
	new GLUI_EditText(panel3, "g (float)", &specularLight.g, 332, voidRecall);
	new GLUI_EditText(panel3, "b (float)", &specularLight.b, 333, voidRecall);
	new GLUI_EditText(panel3, "a (float)", &specularLight.a, 334, voidRecall);
	glui->add_statictext_to_panel(panel3, "");
	glui->add_statictext_to_panel(panel3, "Change from directional light");
	glui->add_statictext_to_panel(panel3, "(At infinity) to point light.");
	new GLUI_Checkbox(panel3, "Directional", &lightAtInfinity, 11, voidRecall);
	glui->add_statictext_to_panel(panel3, "Direction/Position.");
	new GLUI_EditText(panel3, "x (float)", &lightPos[0], 341, voidRecall);
	new GLUI_EditText(panel3, "y (float)", &lightPos[1], 342, voidRecall);
	new GLUI_EditText(panel3, "z (float)", &lightPos[2], 343, voidRecall);

	glui->add_column(true);

	GLUI_Panel *panel4 = new GLUI_Panel(glui, "Camera", GLUI_PANEL_EMBOSSED);
	glui->add_statictext_to_panel(panel4, "Distance of camera to plane.");
	glui->add_statictext_to_panel(panel4, "Equivalent to FOV.");
	new GLUI_EditText(panel4, "e (float)", &e, 41, voidRecall);

	GLUI_Panel *panel5 = new GLUI_Panel(glui, "Screen", GLUI_PANEL_EMBOSSED);
	glui->add_statictext_to_panel(panel5, "Pixels by side.");
	glui->add_statictext_to_panel(panel5, "Pixels = 2m + 1");
	new GLUI_EditText(panel5, "pixels (int)", &l, 51, getM);
}