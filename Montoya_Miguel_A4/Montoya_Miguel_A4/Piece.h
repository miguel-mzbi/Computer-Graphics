#pragma once

#ifndef PIECE_H
#define PIECE_H

/*
Because pieces can be rotated the logic will be managed with offset.
The default configuration, has an offset of 0.
_______ _______
|\ T /| |\ 0 /|
|L X R| |3 X 1|
|/ B \| |/ 2 \|
‾‾‾‾‾‾‾ ‾‾‾‾‾‾‾
An offset of 1 would imply that accesing to the top tringle would actually return the position 0 + 1 (Right)
_______
|\ R /|
|T X B|
|/ L \|
‾‾‾‾‾‾‾
An offset of 2 would imply that accesing to the top tringle would actually return the position 0 + 2 (Bottom)
_______
|\ R /|
|T X B|
|/ L \|
‾‾‾‾‾‾‾
An offset of 3 would imply that accesing to the top tringle would actually return the position 0 + 3 (Left)
_______
|\ R /|
|T X B|
|/ L \|
‾‾‾‾‾‾‾
*/
class Piece {
public:
	int offset;
	int triangles[4];
	float position[2];

	Piece();
	Piece(int top, int right, int bottom, int left, int offset);
	~Piece();

};
#endif



