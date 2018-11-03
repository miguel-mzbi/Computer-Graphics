#include <iostream>
#include "GL/glui.h"
#include "BuildGLUI.h"
#include "Bezier.h"

GLUI *glui;
GLUI_Spinner *spinnerStep;
GLUI_Spinner *spinnerDeCastelT;
GLUI_RadioGroup *radioGroupMode;
extern float step;
extern float deCastelT;
extern int radioSelectedMode;
extern int drawCurve;
extern int drawExtendedCurve;
extern int drawControlPoly;
extern int drawConvezHull;

void emptyCB(int) {
	return;
}

void exitProg(int) {
	exit(0);
}

void clearCallback(int) {
	return;
}

void modeCallback(int) {
	if (radioSelectedMode == 3) {
		spinnerDeCastelT->enable();
	}
	else {
		spinnerDeCastelT->disable();
	}
	glui->sync_live();
}

void updateDegreeCallback(int mode) {
	return;
}

void buildGLUI(int windowID) {

	GLUI_Master.set_glutReshapeFunc(resizeFunction);
	GLUI_Master.set_glutDisplayFunc(displayFunction);
	GLUI_Master.set_glutIdleFunc(idleFunction);
	GLUI_Master.set_glutMouseFunc(mouseFunction);
	glui = GLUI_Master.create_glui_subwindow(windowID, GLUI_SUBWINDOW_LEFT);
	glui->set_main_gfx_window(windowID);

	GLUI_Panel *panelRadio = new GLUI_Panel(glui, "Mode", GLUI_PANEL_EMBOSSED);
	radioGroupMode = glui->add_radiogroup_to_panel(panelRadio, &radioSelectedMode, 10, modeCallback);
	glui->add_radiobutton_to_group(radioGroupMode, "Add control point");
	glui->add_radiobutton_to_group(radioGroupMode, "Move control point");
	glui->add_radiobutton_to_group(radioGroupMode, "Delete control point");
	glui->add_radiobutton_to_group(radioGroupMode, "DeCasteljau");
	spinnerDeCastelT = glui->add_spinner_to_panel(panelRadio, "t = ", GLUI_SPINNER_FLOAT, &deCastelT, 11, emptyCB);
	spinnerDeCastelT->set_float_limits(0.0f, 1.0f, GLUI_LIMIT_CLAMP);
	spinnerDeCastelT->set_speed(0.20f);
	spinnerDeCastelT->disable();

	GLUI_Panel *panelOptions = new GLUI_Panel(glui, "Options", GLUI_PANEL_EMBOSSED);
	spinnerStep = glui->add_spinner_to_panel(panelOptions, "step (%) = ", GLUI_SPINNER_FLOAT, &step, 20, emptyCB);
	spinnerStep->set_float_limits(0.0f, 100.0f, GLUI_LIMIT_CLAMP);
	spinnerStep->set_speed(0.25f);
	glui->add_checkbox_to_panel(panelOptions, "Draw curve", &drawCurve, 21, emptyCB);
	glui->add_checkbox_to_panel(panelOptions, "Draw extended curve", &drawExtendedCurve, 22, emptyCB);
	glui->add_checkbox_to_panel(panelOptions, "Draw control polygon", &drawControlPoly, 23, emptyCB);
	glui->add_checkbox_to_panel(panelOptions, "Draw convex hull", &drawConvezHull, 24, emptyCB);


	
	GLUI_Panel *panelButtons = new GLUI_Panel(glui, "", GLUI_PANEL_NONE);
	new GLUI_Button(panelButtons, "Elevate deree", 30, updateDegreeCallback);
	new GLUI_Button(panelButtons, "Lower degree", 31, updateDegreeCallback);
	new GLUI_Button(panelButtons, "Clear", 32, clearCallback);
	new GLUI_Button(panelButtons, "EXIT", 33, exitProg);
}