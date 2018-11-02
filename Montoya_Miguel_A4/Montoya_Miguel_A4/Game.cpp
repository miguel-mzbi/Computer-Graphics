#include <iostream>
#include <vector>
#include <stdlib.h>
#include <ctime>
#include <time.h>
#include <string>
#include "GL/glui.h"
#include "Piece.h"
#include "Game.h"
#include "BuildGLUI.h"

using namespace std;

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|					Parameters						|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

// Substitutions
int RED = 0;
int YELLOW = 1;
int BLUE = 2;
int X = 0;
int Y = 1;
int CONTAINERS = 0;
int BOARD = 1;

// Time
int minutes = 5;
int seconds = 30;
clock_t start;
int timerOn = 0;
int remainingMins = minutes;
int remainingSecs = seconds;
extern GLUI_StaticText *timeRemaining;

// Basics 
int windowID;
int edge = 500;
extern GLUI *glui;

// Game logic
int BORDER_COLOR = RED;
vector<vector<Piece*>> board(4, vector<Piece*>(6, NULL));
vector<vector<Piece*>> containers(4, vector<Piece*>(6, NULL));
vector<Piece> pieces;
vector<vector<float>> solution(24, vector<float>(3));
int validSolution = 0;
int piecePlaced = 0;
extern GLUI_StaticText *gameStatus;

// Movement
int whichTableOrigin; // 0 means containers, 1 means board
int iOrigin;
int jOrigin;
Piece *selectedPiece;
int movementActive = 0;
float xOrigin;
float yOrigin;
int ctrlPress = 0;

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|					   Utility 						|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

void screenToWorldCoords(int(&screen)[2], float(&world)[2]) {
	int x, y, w, h;
	GLUI_Master.get_viewport_area(&x, &y, &w, &h);

	int deltaH = 0, deltaW = 0, ed = 0;
	if (w > h) ed = h;
	else ed = w;
	deltaH = abs(h - ed) / 2;
	deltaW = abs(w - ed) / 2;

	float wx = 2 * (((screen[X] - (x + deltaW)) / (float)ed) - 0.5f);
	float wy = -2 * (((screen[Y] - (y + deltaH)) / (float)ed) - 0.5f);
	world[X] = wx;
	world[Y] = wy;
}

void cleanVariables() {
	selectedPiece = NULL;
}

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|					   Game 						|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void endGame() {
	timerOn = 0;
	GLUI_Master.set_glutMouseFunc(NULL);
}

void checkUserSolution() {
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 6; i++) {
			// Not all pieces used (Incomplete by definition)
			if (board[j][i] == NULL) {
				return;
			}
			Piece current = *board[j][i];
			// If piece is on the left side
			if (i == 0) {
				// If low corner
				if (j == 0) {
					// If piece left and bottom side aren't the same color as the border return
					if (current.triangles[(3 + current.offset) % 4] != BORDER_COLOR 
						|| current.triangles[(2 + current.offset) % 4] != BORDER_COLOR) {
						return;
					}
					// Check colors with adjacent pieces
					if (board[j + 1][i] == NULL || board[j][i + 1] == NULL
						|| current.triangles[(0 + current.offset) % 4] != board[j + 1][i]->triangles[(2 + board[j + 1][i]->offset)%4]
						|| current.triangles[(1 + current.offset) % 4] != board[j][i + 1]->triangles[(3 + board[j][i + 1]->offset)%4]) {
						return;
					}
				}
				// If up corner
				else if (j == 3) {
					// If piece left and upper side aren't the same color as the border return
					if (current.triangles[(0 + current.offset) % 4] != BORDER_COLOR
						|| current.triangles[(3 + current.offset) % 4] != BORDER_COLOR) {
						return;
					}
					// Check colors with adjacent pieces
					if (board[j - 1][i] == NULL || board[j][i + 1] == NULL
						|| current.triangles[(1 + current.offset) % 4] != board[j][i + 1]->triangles[(3 + board[j][i + 1]->offset)%4]
						|| current.triangles[(2 + current.offset) % 4] != board[j - 1][i]->triangles[(0 + board[j - 1][i]->offset)%4]) {
						return;
					}
				}
				// Not a corner
				else {
					// If piece is left side isn't the same color as the border return
					if (current.triangles[(3 + current.offset) % 4] != BORDER_COLOR) {
						return;
					}
					// Check colors with adjacent pieces
					if (board[j + 1][i] == NULL || board[j][i + 1] == NULL || board[j - 1][i] == NULL
						|| current.triangles[(0 + current.offset) % 4] != board[j + 1][i]->triangles[(2 + board[j + 1][i]->offset)%4]
						|| current.triangles[(1 + current.offset) % 4] != board[j][i + 1]->triangles[(3 + board[j][i + 1]->offset)%4]
						|| current.triangles[(2 + current.offset) % 4] != board[j - 1][i]->triangles[(0 + board[j - 1][i]->offset)%4]) {
						return;
					}
				}
			}
			// If piece is on the right side
			else if (i == 5) {
				// If low corner
				if (j == 0) {
					// If piece right and bottom side aren't the same color as the border return
					if (current.triangles[(1 + current.offset) % 4] != BORDER_COLOR
						|| current.triangles[(2 + current.offset) % 4] != BORDER_COLOR) {
						return;
					}
					// Check colors with adjacent pieces
					if (board[j + 1][i] == NULL || board[j][i - 1] == NULL
						|| current.triangles[(0 + current.offset) % 4] != board[j + 1][i]->triangles[(2 + board[j + 1][i]->offset)%4]
						|| current.triangles[(3 + current.offset) % 4] != board[j][i - 1]->triangles[(1 + board[j][i - 1]->offset)%4]) {
						return;
					}
				}
				// If up corner
				else if (j == 3) {
					// If piece right and upper side aren't the same color as the border return
					if (current.triangles[(0 + current.offset) % 4] != BORDER_COLOR
						|| current.triangles[(1 + current.offset) % 4] != BORDER_COLOR) {
						return;
					}
					// Check colors with adjacent pieces
					if (board[j - 1][i] == NULL || board[j][i - 1] == NULL
						|| current.triangles[(3 + current.offset) % 4] != board[j][i - 1]->triangles[(1 + board[j][i - 1]->offset)%4]
						|| current.triangles[(2 + current.offset) % 4] != board[j - 1][i]->triangles[(0 + board[j - 1][i]->offset)%4]) {
						return;
					}
				}
				// Not a corner
				else {
					// If piece is right side isn't the same color as the border return
					if (current.triangles[(1 + current.offset) % 4] != BORDER_COLOR) {
						return;
					}
					// Check colors with adjacent pieces
					if (board[j + 1][i] == NULL || board[j][i - 1] == NULL || board[j - 1][i] == NULL
						|| current.triangles[(0 + current.offset) % 4] != board[j + 1][i]->triangles[(2 + board[j + 1][i]->offset)%4]
						|| current.triangles[(3 + current.offset) % 4] != board[j][i - 1]->triangles[(1 + board[j][i - 1]->offset)%4]
						|| current.triangles[(2 + current.offset) % 4] != board[j - 1][i]->triangles[(0 + board[j - 1][i]->offset)%4]) {
						return;
					}
				}
			}
			// If middle piece
			else {
				// If piece is in the bottom
				if (j == 0) {
					// If piece bottom side isn't the same color as the border return
					if (current.triangles[(2 + current.offset) % 4] != BORDER_COLOR) {
						return;
					}
					// Check colors with adjacent pieces
					if (board[j + 1][i] == NULL || board[j][i + 1] == NULL || board[j][i - 1] == NULL
						|| current.triangles[(0 + current.offset) % 4] != board[j + 1][i]->triangles[(2 + board[j + 1][i]->offset)%4]
						|| current.triangles[(1 + current.offset) % 4] != board[j][i + 1]->triangles[(3 + board[j][i + 1]->offset)%4]
						|| current.triangles[(3 + current.offset) % 4] != board[j][i - 1]->triangles[(1 + board[j][i - 1]->offset)%4]) {
						return;
					}
				}
				// If piece is in the top
				else if (j == 3) {
					// If piece top side isn't the same color as the border return
					if (current.triangles[(0 + current.offset) % 4] != BORDER_COLOR) {
						return;
					}
					// Check colors with adjacent pieces
					if (board[j - 1][i] == NULL || board[j][i - 1] == NULL || board[j][i + 1] == NULL
						|| current.triangles[(1 + current.offset) % 4] != board[j][i + 1]->triangles[(3 + board[j][i + 1]->offset)%4]
						|| current.triangles[(2 + current.offset) % 4] != board[j - 1][i]->triangles[(0 + board[j - 1][i]->offset)%4]
						|| current.triangles[(3 + current.offset) % 4] != board[j][i - 1]->triangles[(1 + board[j][i - 1]->offset)%4]) {
						return;
					}
				}
				// Piece doesn't touch any border at all
				else {
					// Check colors with adjacent pieces
					if (board[j + 1][i] == NULL || board[j][i + 1] == NULL || board[j - 1][i] == NULL || board[j][i - 1] == NULL
						|| current.triangles[(0 + current.offset) % 4] != board[j + 1][i]->triangles[(2 + board[j + 1][i]->offset)%4]
						|| current.triangles[(1 + current.offset) % 4] != board[j][i + 1]->triangles[(3 + board[j][i + 1]->offset)%4]
						|| current.triangles[(2 + current.offset) % 4] != board[j - 1][i]->triangles[(0 + board[j - 1][i]->offset)%4]
						|| current.triangles[(3 + current.offset) % 4] != board[j][i - 1]->triangles[(1 + board[j][i - 1]->offset)%4]) {
						return;
					}
				}
			}
		}
	}
	validSolution = 1;
}

void createSolution() {
	solution[22][X] = 0;
	solution[22][Y] = 0;
	solution[22][2] = 2;
	solution[3][X] = 1;
	solution[3][Y] = 0;
	solution[3][2] = 0;
	solution[14][X] = 2;
	solution[14][Y] = 0;
	solution[14][2] = 2;
	solution[15][X] = 3;
	solution[15][Y] = 0;
	solution[15][2] = 3;
	solution[13][X] = 4;
	solution[13][Y] = 0;
	solution[13][2] = 1;
	solution[21][X] = 5;
	solution[21][Y] = 0;
	solution[21][2] = 3;

	solution[18][X] = 0;
	solution[18][Y] = 1;
	solution[18][2] = 1;
	solution[19][X] = 1;
	solution[19][Y] = 1;
	solution[19][2] = 0;
	solution[8][X] = 2;
	solution[8][Y] = 1;
	solution[8][2] = 0;
	solution[11][X] = 3;
	solution[11][Y] = 1;
	solution[11][2] = 1;
	solution[10][X] = 4;
	solution[10][Y] = 1;
	solution[10][2] = 2;

	solution[5][X] = 5;
	solution[5][Y] = 1;
	solution[5][2] = 3;
	solution[4][X] = 0;
	solution[4][Y] = 2;
	solution[4][2] = 2;
	solution[23][X] = 1;
	solution[23][Y] = 2;
	solution[23][2] = 2;
	solution[12][X] = 2;
	solution[12][Y] = 2;
	solution[12][2] = 2;
	solution[0][X] = 3;
	solution[0][Y] = 2;
	solution[0][2] = 0;
	solution[9][X] = 4;
	solution[9][Y] = 2;
	solution[9][2] = 3;
	solution[7][X] = 5;
	solution[7][Y] = 2;
	solution[7][2] = 1;

	solution[16][X] = 0;
	solution[16][Y] = 3;
	solution[16][2] = 0;
	solution[20][X] = 1;
	solution[20][Y] = 3;
	solution[20][2] = 2;
	solution[17][X] = 2;
	solution[17][Y] = 3;
	solution[17][2] = 0;
	solution[6][X] = 3;
	solution[6][Y] = 3;
	solution[6][2] = 1;
	solution[1][X] = 4;
	solution[1][Y] = 3;
	solution[1][2] = 0;
	solution[2][X] = 5;
	solution[2][Y] = 3;
	solution[2][2] = 0;

	board = vector<vector<Piece*>>(4, vector<Piece*>(6, NULL));
	containers = vector<vector<Piece*>>(4, vector<Piece*>(6, NULL));
	selectedPiece = NULL;

	for (int i = 0; i < 24; i++) {
		pieces[i].position[X] = -0.6f + solution[i][X] * 0.2f;
		pieces[i].position[Y] = 0.1f + solution[i][Y] * 0.2f;
		pieces[i].offset = (int)solution[i][2];

		board[solution[i][Y]][solution[i][X]] = &pieces[i];
	}

	glutPostRedisplay();
}

void createPieces() {

	pieces.push_back(Piece(YELLOW, YELLOW, YELLOW, YELLOW, rand() % 4));
	pieces.push_back(Piece(RED, YELLOW, YELLOW, YELLOW, rand() % 4));
	pieces.push_back(Piece(RED, RED, YELLOW, YELLOW, rand() % 4));
	pieces.push_back(Piece(RED, YELLOW, RED, YELLOW, rand() % 4));
	pieces.push_back(Piece(RED, RED, RED, YELLOW, rand() % 4));
	pieces.push_back(Piece(RED, BLUE, YELLOW, YELLOW, rand() % 4));
	pieces.push_back(Piece(BLUE, RED, YELLOW, YELLOW, rand() % 4));
	pieces.push_back(Piece(BLUE, YELLOW, RED, YELLOW, rand() % 4));

	pieces.push_back(Piece(BLUE, BLUE, BLUE, BLUE, rand() % 4));
	pieces.push_back(Piece(BLUE, YELLOW, YELLOW, YELLOW, rand() % 4));
	pieces.push_back(Piece(BLUE, BLUE, YELLOW, YELLOW, rand() % 4));
	pieces.push_back(Piece(BLUE, YELLOW, BLUE, YELLOW, rand() % 4));
	pieces.push_back(Piece(BLUE, BLUE, BLUE, YELLOW, rand() % 4));
	pieces.push_back(Piece(BLUE, BLUE, YELLOW, RED, rand() % 4));
	pieces.push_back(Piece(RED, YELLOW, BLUE, BLUE, rand() % 4));
	pieces.push_back(Piece(BLUE, RED, BLUE, YELLOW, rand() % 4));

	pieces.push_back(Piece(RED, RED, RED, RED, rand() % 4));
	pieces.push_back(Piece(RED, BLUE, BLUE, BLUE, rand() % 4));
	pieces.push_back(Piece(RED, RED, BLUE, BLUE, rand() % 4));
	pieces.push_back(Piece(RED, BLUE, RED, BLUE, rand() % 4));
	pieces.push_back(Piece(RED, RED, RED, BLUE, rand() % 4));
	pieces.push_back(Piece(RED, RED, YELLOW, BLUE, rand() % 4));
	pieces.push_back(Piece(RED, RED, BLUE, YELLOW, rand() % 4));
	pieces.push_back(Piece(RED, YELLOW, RED, BLUE, rand() % 4));

}

void drawBoard() {

	for (float i = -0.6f; i <= 0.6f; i+= 0.2f) {
		glColor3f(1.0f, 1.0f, 1.0f);
		glLineWidth(1);
		if (i == -0.6f || i > 0.59f) {
			glLineWidth(3);
			if (BORDER_COLOR == YELLOW) glColor3f(1.0f, 1.0f, 0.0f);
			else if (BORDER_COLOR == RED) glColor3f(1.0f, 0.0f, 0.0f);
			else if (BORDER_COLOR == BLUE) glColor3f(0.0f, 0.0f, 1.0f);
		}
		glBegin(GL_LINES);
		glVertex2f(i, 0.9f);
		glVertex2f(i, 0.1f);
		glEnd();

	}

	for (float j = 0.1f; j <= 0.9f; j += 0.2f) {
		glColor3f(1.0f, 1.0f, 1.0f);
		glLineWidth(1);
		if (j == 0.1f || j >= 0.9f) {
			glLineWidth(3);
			if (BORDER_COLOR == YELLOW) glColor3f(1.0f, 1.0f, 0.0f);
			else if (BORDER_COLOR == RED) glColor3f(1.0f, 0.0f, 0.0f);
			else if (BORDER_COLOR == BLUE) glColor3f(0.0f, 0.0f, 1.0f);
		}
		glBegin(GL_LINES);
		glVertex2f(-0.6f, j);
		glVertex2f(0.6f, j);
		glEnd();
	}


}

void drawPieceCointainers() {
	glColor3f(1.0f, 1.0f, 1.0f);
	glLineWidth(3);

	glBegin(GL_LINES);

	for (float j = -1.0f; j < 0.0f; j += 0.25f) {
		float myY = j + 0.025f;
		for (float i = -1.0f; i < 1.0f; i += 2.0f / 6.0f) {
			float myX = i + ((2.0f / 6.0f) - 0.2f) / 2.0f;
			glVertex2f(myX, myY);
			glVertex2f(myX + 0.2f, myY);
			glVertex2f(myX, myY);
			glVertex2f(myX, myY + 0.2f);
			glVertex2f(myX + 0.2f, myY);
			glVertex2f(myX + 0.2f, myY + 0.2f);
			glVertex2f(myX, myY + 0.2f);
			glVertex2f(myX + 0.2f, myY + 0.2f);
		}
	}

	glEnd();

}

void putPiecesOnContainers() {
	for (int j = 0; j < 4; j++) {
		float myY = (-1.0f + j*0.25f) + 0.025f;

		for (int i = 0; i < 6; i++) {
			float myX = (-1.0f + i * (2.0f / 6.0f)) + ((2.0f / 6.0f) - 0.2f) / 2.0f;

			Piece *myPiece = &pieces[j * 6 + i];
			myPiece->position[X] = myX;
			myPiece->position[Y] = myY;
			containers[j][i] = &pieces[j * 6 + i];

		}
	}
}

void restart() {
	GLUI_Master.set_glutMouseFunc(mouseFunction);
	gameStatus->set_text("");
	remainingMins = minutes;
	remainingSecs = seconds;
	string text;
	if (remainingSecs < 10) {
		text = "Remaing time: " + to_string(remainingMins) + ":0" + to_string(remainingSecs);
	}
	else {
		text = "Remaing time: " + to_string(remainingMins) + ":" + to_string(remainingSecs);
	}
	timeRemaining->set_text(text.c_str());
	glui->sync_live();

	validSolution = 0;
	board = vector<vector<Piece*>>(4, vector<Piece*>(6, NULL));
	containers = vector<vector<Piece*>>(4, vector<Piece*>(6, NULL));
	selectedPiece = NULL;
	putPiecesOnContainers();
	glutPostRedisplay();
}

void drawAllPieces() {
	glBegin(GL_TRIANGLES);

	for (unsigned int i = 0; i < pieces.size(); i++) {
		Piece *myPiece = &pieces[i];
		float center[2];
		float myX = myPiece->position[X];
		float myY = myPiece->position[Y];
		center[X] = myX + 0.1f;
		center[Y] = myY + 0.1f;

		// Top
		if (myPiece->triangles[(0 + myPiece->offset) % 4] == YELLOW) glColor3f(1.0f, 1.0f, 0.0f);
		else if (myPiece->triangles[(0 + myPiece->offset) % 4] == RED) glColor3f(1.0f, 0.0f, 0.0f);
		else if (myPiece->triangles[(0 + myPiece->offset) % 4] == BLUE) glColor3f(0.0f, 0.0f, 1.0f);
		glVertex2f(myX, myY + 0.2f);
		glVertex2f(myX + 0.2f, myY + 0.2f);
		glVertex2f(center[X], center[Y]);
		// Right
		if (myPiece->triangles[(1 + myPiece->offset) % 4] == YELLOW) glColor3f(1.0f, 1.0f, 0.0f);
		else if (myPiece->triangles[(1 + myPiece->offset) % 4] == RED) glColor3f(1.0f, 0.0f, 0.0f);
		else if (myPiece->triangles[(1 + myPiece->offset) % 4] == BLUE) glColor3f(0.0f, 0.0f, 1.0f);
		glVertex2f(myX + 0.2f, myY);
		glVertex2f(myX + 0.2f, myY + 0.2f);
		glVertex2f(center[X], center[Y]);
		// Bottom
		if (myPiece->triangles[(2 + myPiece->offset) % 4] == YELLOW) glColor3f(1.0f, 1.0f, 0.0f);
		else if (myPiece->triangles[(2 + myPiece->offset) % 4] == RED) glColor3f(1.0f, 0.0f, 0.0f);
		else if (myPiece->triangles[(2 + myPiece->offset) % 4] == BLUE) glColor3f(0.0f, 0.0f, 1.0f);
		glVertex2f(myX, myY);
		glVertex2f(myX + 0.2f, myY);
		glVertex2f(center[X], center[Y]);
		// Left
		if (myPiece->triangles[(3 + myPiece->offset) % 4] == YELLOW) glColor3f(1.0f, 1.0f, 0.0f);
		else if (myPiece->triangles[(3 + myPiece->offset) % 4] == RED) glColor3f(1.0f, 0.0f, 0.0f);
		else if (myPiece->triangles[(3 + myPiece->offset) % 4] == BLUE) glColor3f(0.0f, 0.0f, 1.0f);
		glVertex2f(myX, myY);
		glVertex2f(myX, myY + 0.2f);
		glVertex2f(center[X], center[Y]);
	}

	glEnd();
}

int getPiece(float (&world)[2], Piece **selectedPiece, int &i, int &j, int &whichTable) {
	float wy = world[Y];
	float wx = world[X];

	// Clicked over containers
	if (wy < 0) {
		if (-0.975f <= wy && wy <= -0.775f) j = 0;
		else if (-0.725f <= wy && wy <= -0.525f) j = 1;
		else if (-0.475f <= wy && wy <= -0.275f) j = 2;
		else if (-0.225f <= wy && wy <= -0.025f) j = 3;
		else j = -1;

		if (-0.933f <= wx && wx <= -0.733) i = 0;
		else if (-0.6f <= wx && wx <= -0.4) i = 1;
		else if (-0.266f <= wx && wx <= -0.066) i = 2;
		else if (0.066f <= wx && wx <= 0.266) i = 3;
		else if (0.4f <= wx && wx <= 0.6) i = 4;
		else if (0.733f <= wx && wx <= 0.933) i = 5;
		else i = -1;

		if (j == -1 || i == -1) {
			return 0;
		}

		if (containers[j][i] != NULL) {
			*selectedPiece = containers[j][i];
		}
		whichTable = CONTAINERS;
	}
	else {
		i = -1;
		for (int a = 0; a < 6; a++) {
			if (-0.6f + a * 0.2f <= wx && wx <= -0.6f + a * 0.2f + 0.2f) {
				i = a;
				break;
			}
		}

		j = -1;
		for (int b = 0; b < 4; b++) {
			if (0.1f + b * 0.2f <= wy && wy <= 0.1f + b * 0.2f + 0.2f) {
				j = b;
				break;
			}
		}

		if (j == -1 || i == -1) {
			return 0;
		}

		if (board[j][i] != NULL) {
			*selectedPiece = board[j][i];
		}
		whichTable = BOARD;
	}
	return 1;
}

void setPieceLocation(int whichBoard, int i, int j, Piece **piece) {
	if (whichBoard == CONTAINERS) {
		(*piece)->position[X] = (-1.0f + i * (2.0f / 6.0f)) + ((2.0f / 6.0f) - 0.2f) / 2.0f;
		(*piece)->position[Y] = (-1.0f + j * 0.25f) + 0.025f;
	}
	else {
		(*piece)->position[X] = -0.6f + i * 0.2f;
		(*piece)->position[Y] = 0.1f + j * 0.2f;
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

	drawBoard();
	drawPieceCointainers();

	drawAllPieces();

	if (piecePlaced) {
		piecePlaced = 0;
		checkUserSolution();
		if (validSolution) {
			cout << "User won!\n";
			gameStatus->set_text("USER WON!");
			endGame();
		}
	}

	glFlush();
}

void idleFunction() {
	if (glutGetWindow() != windowID) glutSetWindow(windowID);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFlush();

	drawBoard();
	drawPieceCointainers();

	drawAllPieces();

	if (timerOn) {
		double duration;
		duration = (clock() - start) / (double)CLOCKS_PER_SEC;
		if (duration >= minutes * 60 + seconds) {
			cout << "Time ended\n";
			gameStatus->set_text("Time ended");
			checkUserSolution();
			if (!validSolution) {
				createSolution();
			}
			else {
				cout << "User Won!\n";
				gameStatus->set_text("USER WON!");
			}
			endGame();
		}
		int rem = minutes * 60 + seconds - (int)duration;
		remainingMins = rem / 60;
		remainingSecs = rem % 60;
		string text;
		if (remainingSecs < 10) {
			text = "Remaing time: " + to_string(remainingMins) + ":0" + to_string(remainingSecs);
		}
		else {
			text = "Remaing time: " + to_string(remainingMins) + ":" + to_string(remainingSecs);
		}
		timeRemaining->set_text(text.c_str());
		glui->sync_live();
	}

	glFlush();
}

void resizeFunction(int width, int height) {
	if (width == 0 || height == 0) return;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	int x, y, w, h;
	GLUI_Master.get_viewport_area(&x, &y, &w, &h);

	int deltaH = 0, deltaW = 0, ed = 0;
	if (w > h) ed = h;
	else ed = w;

	deltaH = abs(h - ed) / 2;
	deltaW = abs(w - ed) / 2;
	glViewport(x + deltaW, y + deltaH, ed, ed);
}

void mouseFunction(int button, int state, int x, int y) {
	// If is not left button return
	if (button != GLUT_LEFT_BUTTON) return;
	if (state == GLUT_DOWN) {
		selectedPiece = NULL;
		int screen[] = { x, y };
		float world[2];
		screenToWorldCoords(screen, world);
		// If out of borders return
		if (world[X] > 1.0f || world[X] < -1.0f || world[Y] > 1.0f || world[Y] < -1.0f) return;
		
		if (getPiece(world, &selectedPiece, iOrigin, jOrigin, whichTableOrigin)) {
			// If no piece was selected return
			if (selectedPiece == NULL) return;
			if (glutGetModifiers() == GLUT_ACTIVE_CTRL) {
				selectedPiece->offset = (selectedPiece->offset +1) % 4;
			}
			xOrigin = selectedPiece->position[X];
			yOrigin = selectedPiece->position[Y];
			movementActive = 1;
		}
	}
	else {
		movementActive = 0;
		// If nothing was selected return
		if (selectedPiece == NULL) return;
		int screen[] = { x, y };
		float world[2];
		screenToWorldCoords(screen, world);
		// If out of borders return and return piece to origin
		if (world[X] > 1.0f || world[X] < -1.0f || world[Y] > 1.0f || world[Y] < -1.0f) {
			if (selectedPiece != NULL) {
				selectedPiece->position[X] = xOrigin;
				selectedPiece->position[Y] = yOrigin;
			}
			return;
		}

		Piece *selectedPieceDestination = NULL;
		int i, j, whichTableDestination;
		if (getPiece(world, &selectedPieceDestination, i, j, whichTableDestination)) {
			// If no piece in destination move
			if (selectedPieceDestination == NULL) {
				// Place destination is board
				if (whichTableDestination == BOARD) {
					// Place origin is board
					if (whichTableOrigin == BOARD) {
						board[jOrigin][iOrigin] = NULL;
						board[j][i] = selectedPiece;
					}
					// Place origin is containeru
					else {
						containers[jOrigin][iOrigin] = NULL;
						board[j][i] = selectedPiece;
					}
					piecePlaced = 1;
				}
				// Place destination is container
				else {
					// Place origin is board
					if (whichTableOrigin == BOARD) {
						board[jOrigin][iOrigin] = NULL;
						containers[j][i] = selectedPiece;
					}
					// Place origin is container
					else {
						containers[jOrigin][iOrigin] = NULL;
						containers[j][i] = selectedPiece;
					}
				}
				setPieceLocation(whichTableDestination, i, j, &selectedPiece);
			}
			// Return to original place
			else if (selectedPiece != NULL) {
				selectedPiece->position[X] = xOrigin;
				selectedPiece->position[Y] = yOrigin;
				return;
			}
		}
		else if (selectedPiece != NULL) {
			selectedPiece->position[X] = xOrigin;
			selectedPiece->position[Y] = yOrigin;
			return;
		}
	}
}

void motionFunction(int x, int y) {
	if (movementActive && selectedPiece != NULL) {
		if (glutGetModifiers() == GLUT_ACTIVE_CTRL && !ctrlPress) {
			selectedPiece->offset = (selectedPiece->offset + 1) % 4;
			ctrlPress = 1;
		}
		else if (glutGetModifiers() != GLUT_ACTIVE_CTRL) {
			ctrlPress = 0;
		}
		int screen[] = { x, y };
		float world[2];
		screenToWorldCoords(screen, world);
		// If out of borders return
		if (world[X] > 1.0f || world[X] < -1.0f || world[Y] > 1.0f || world[Y] < -1.0f) return;
		selectedPiece->position[X] = world[X]-0.1f;
		selectedPiece->position[Y] = world[Y]-0.1f;
		displayFunction();
	}
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
	glutInitWindowPosition(300, 300);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	windowID = glutCreateWindow("Montoya_Miguel A3");
	glutSetWindow(windowID);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Set up callbacks (All done by GLUI, except motion)
	//glutDisplayFunc(displayFunction);
	glutMotionFunc(motionFunction);

	// Create GUI
	buildGLUI(windowID);

	// Game setup
	srand((unsigned int)time(0));
	createPieces();
	putPiecesOnContainers();

	// Start loop
	glutMainLoop();

	return 0;
}