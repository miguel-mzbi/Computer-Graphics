#pragma once

class Buffer {
public:
	unsigned char *buffer;
	int w;
	int h;

	Buffer();
	Buffer(int w, int h);

	void drawPixel(int x, int y, Color const &color);
	void drawBuffer();
};