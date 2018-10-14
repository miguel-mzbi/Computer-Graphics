#include "GL/freeglut.h"
#include "Color.h"
#include "Buffer.h"

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|			Pixel's buffer. Pixels to draw.			|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

// Declaration
Buffer::Buffer() {}

// Init
Buffer::Buffer(int w, int h) {
	this->w = w;
	this->h = h;
	this->buffer = (unsigned char *)malloc(w*h * 3 * sizeof(unsigned char));
}

// Store pixel values
void Buffer::drawPixel(int x, int y, Color const &c) {
	if ((x < 0) || (x >= w) || (y < 0) || (y >= h)) return;
	this->buffer[3 * (this->w * y + x) + 0] = (unsigned char)(255 * c.r);
	this->buffer[3 * (this->w * y + x) + 1] = (unsigned char)(255 * c.g);
	this->buffer[3 * (this->w * y + x) + 2] = (unsigned char)(255 * c.b);
}

// Draw buffer
void Buffer::drawBuffer() {
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glRasterPos4f(-1.0, -1.0, 0, 1);
	glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer);
}