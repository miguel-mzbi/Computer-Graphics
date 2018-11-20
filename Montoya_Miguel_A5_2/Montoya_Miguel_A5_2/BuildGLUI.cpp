#include <iostream>
#include "GL/glui.h"
#include "BuildGLUI.h"
#include "Bezier.h"

GLUI *glui;
GLUI_Spinner *spinnerStep;
GLUI_Spinner *spinnerDeCastelT;
GLUI_RadioGroup *radioGroupMode;
GLUI_Button *decreaseButton;
extern int *segNum;
extern float *deCastelT;
extern int *radioSelectedMode;
extern int *drawCurve;
extern int *drawExtendedCurve;
extern int *drawControlPoly;
extern int *drawHull;
extern int selectedCurve;

extern void increaseOrder();
extern void decreaseOrder();
extern void clearScreen();
extern void changeCurve();
extern void applyChanges();

void emptyCB(int) {
	applyChanges();
	return;
}

void changeSelectedCurve(int) {
	decreaseButton->disable();
	changeCurve();
	if (*radioSelectedMode == 3) {
		spinnerDeCastelT->enable();
	}
	else {
		spinnerDeCastelT->disable();
	}
	glui->sync_live();
}

void exitProg(int) {
	exit(0);
}

void clearCallback(int) {
	clearScreen();
	decreaseButton->disable();
}

void modeCallback(int) {
	applyChanges();
	if (*radioSelectedMode == 3) {
		spinnerDeCastelT->enable();
	}
	else {
		spinnerDeCastelT->disable();
	}
	glui->sync_live();
}

void updateDegreeCallback(int mode) {
	if (mode == 30) {
		increaseOrder();
		decreaseButton->enable();
	}
	else if (mode == 31) {
		decreaseOrder();
		decreaseButton->disable();
	}
}

void buildGLUI(int mainWindowID) {

	GLUI_Master.set_glutReshapeFunc(resizeFunction);
	GLUI_Master.set_glutDisplayFunc(displayFunction);
	GLUI_Master.set_glutIdleFunc(idleFunction);
	GLUI_Master.set_glutMouseFunc(mouseFunction);
	glui = GLUI_Master.create_glui_subwindow(mainWindowID, GLUI_SUBWINDOW_LEFT);
	glui->set_main_gfx_window(mainWindowID);

	GLUI_RadioGroup *radioGroupCurve = glui->add_radiogroup(&selectedCurve, 0, changeSelectedCurve);
	glui->add_radiobutton_to_group(radioGroupCurve, "Curve 1");
	glui->add_radiobutton_to_group(radioGroupCurve, "Curve 2");

	GLUI_Panel *panelRadio = new GLUI_Panel(glui, "Mode", GLUI_PANEL_EMBOSSED);
	radioGroupMode = glui->add_radiogroup_to_panel(panelRadio, radioSelectedMode, 10, modeCallback);
	glui->add_radiobutton_to_group(radioGroupMode, "Add control point");
	glui->add_radiobutton_to_group(radioGroupMode, "Move control point");
	glui->add_radiobutton_to_group(radioGroupMode, "Delete control point");
	glui->add_radiobutton_to_group(radioGroupMode, "DeCasteljau");
	spinnerDeCastelT = glui->add_spinner_to_panel(panelRadio, "t = ", GLUI_SPINNER_FLOAT, deCastelT, 11, emptyCB);
	spinnerDeCastelT->set_float_limits(0.0f, 1.0f, GLUI_LIMIT_CLAMP);
	spinnerDeCastelT->set_speed(1.0f);
	spinnerDeCastelT->disable();

	GLUI_Panel *panelOptions = new GLUI_Panel(glui, "Options", GLUI_PANEL_EMBOSSED);
	spinnerStep = glui->add_spinner_to_panel(panelOptions, "Segment No = ", GLUI_SPINNER_INT, segNum, 20, emptyCB);
	spinnerStep->set_int_limits(1, 100000, GLUI_LIMIT_CLAMP);
	spinnerStep->set_speed(0.25f);
	glui->add_checkbox_to_panel(panelOptions, "Draw curve", drawCurve, 21, emptyCB);
	glui->add_checkbox_to_panel(panelOptions, "Draw extended curve", drawExtendedCurve, 22, emptyCB);
	glui->add_checkbox_to_panel(panelOptions, "Draw control polygon", drawControlPoly, 23, emptyCB);
	glui->add_checkbox_to_panel(panelOptions, "Draw convex hull", drawHull, 24, emptyCB);
	
	GLUI_Panel *panelButtons = new GLUI_Panel(glui, "", GLUI_PANEL_NONE);
	new GLUI_Button(panelButtons, "Elevate deree", 30, updateDegreeCallback);
	decreaseButton = new GLUI_Button(panelButtons, "Lower degree", 31, updateDegreeCallback);
	new GLUI_Button(panelButtons, "Clear", 32, clearCallback);
	new GLUI_Button(panelButtons, "EXIT", 33, exitProg);
	new GLUI_StaticText(panelButtons, "");
	new GLUI_StaticText(panelButtons, "There can only be at");
	new GLUI_StaticText(panelButtons, "most 32 control points");


}