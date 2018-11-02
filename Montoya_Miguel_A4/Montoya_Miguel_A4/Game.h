#pragma once

#ifndef GAME_H
#define GAME_H

void displayFunction();
void idleFunction();
void resizeFunction(int width, int height);
void mouseFunction(int button, int state, int x, int y);
void restart();
#endif