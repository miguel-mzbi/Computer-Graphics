#include "Piece.h"


Piece::Piece() {}

Piece::Piece(int top, int right, int bottom, int left, int offset) {
	this->offset = 0;
	this->triangles[0] = top;
	this->triangles[1] = right;
	this->triangles[2] = bottom;
	this->triangles[3] = left;
	this->offset = offset;
}

Piece::~Piece(){}
