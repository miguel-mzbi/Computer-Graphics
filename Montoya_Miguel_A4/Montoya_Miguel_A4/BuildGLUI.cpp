#include <ctime>
#include <iostream>
#include <string>
#include "GL/glui.h"
#include "BuildGLUI.h"
#include "Game.h"

GLUI *glui;
GLUI_Spinner *spinnerMin;
GLUI_Spinner *spinnerSec;
GLUI_StaticText *timeRemaining;
GLUI_StaticText *gameStatus;
GLUI_Button *buttonStart;
extern int minutes;
extern int seconds;
extern clock_t start;
extern int timerOn;


void exitGame(int) {
	exit(0);
}

void startOverGame(int) {
	timerOn = 0;
	spinnerMin->enable();
	spinnerSec->enable();
	buttonStart->enable();
	restart();
}

void checkTime(int) { 
	std::string text;
	if (seconds < 10) {
		text = "Remaing time: " + std::to_string(minutes) + ":0" + std::to_string(seconds);
	}
	else {
		text = "Remaing time: " + std::to_string(minutes) + ":" + std::to_string(seconds);
	}
	timeRemaining->set_text(text.c_str());
	glui->sync_live();
}

void startTimer(int) {
	if (timerOn) return;
	start = clock();
	timerOn = 1;
	spinnerMin->disable();
	spinnerSec->disable();
	buttonStart->disable();
	glui->sync_live();
}

void buildGLUI(int windowID) {

	GLUI_Master.set_glutReshapeFunc(resizeFunction);
	GLUI_Master.set_glutDisplayFunc(displayFunction);
	GLUI_Master.set_glutIdleFunc(idleFunction);
	GLUI_Master.set_glutMouseFunc(mouseFunction);
	glui = GLUI_Master.create_glui_subwindow(windowID, GLUI_SUBWINDOW_LEFT);
	glui->set_main_gfx_window(windowID);

	GLUI_Panel *panelTime = new GLUI_Panel(glui, "Time", GLUI_PANEL_EMBOSSED);
	spinnerMin = new GLUI_Spinner(panelTime, "Mins", GLUI_SPINNER_INT, &minutes, 01, checkTime);
	spinnerMin->set_int_limits(0, 59, GLUI_LIMIT_CLAMP);
	spinnerMin->set_speed(0.25f);
	spinnerSec = new GLUI_Spinner(panelTime, "Secs", GLUI_SPINNER_INT, &seconds, 02, checkTime);
	spinnerSec->set_int_limits(0, 59, GLUI_LIMIT_CLAMP);
	spinnerSec->set_speed(0.25f);
	buttonStart = new GLUI_Button(panelTime, "Start Timer", 03, startTimer);
	std::string text;
	if (seconds < 10) {
		text = "Remaing time: " + std::to_string(minutes) + ":0" + std::to_string(seconds);
	}
	else {
		text = "Remaing time: " + std::to_string(minutes) + ":" + std::to_string(seconds);
	}	
	new GLUI_StaticText(panelTime, "");
	timeRemaining = new GLUI_StaticText(panelTime, text.c_str());

	GLUI_Panel *panel = new GLUI_Panel(glui, "", GLUI_PANEL_NONE);
	new GLUI_Button(panel, "Start Over", 10, startOverGame);
	new GLUI_Button(panel, "EXIT", 11, exitGame);

	GLUI_Panel *panelStatus = new GLUI_Panel(glui, "", GLUI_PANEL_NONE);
	new GLUI_StaticText(panelStatus, "");
	gameStatus = new GLUI_StaticText(panelStatus, "");
}