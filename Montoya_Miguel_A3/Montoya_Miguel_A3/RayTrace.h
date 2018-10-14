#pragma once

#ifndef RAYTRACE_H
#define RAYTRACE_H

void calculateDirection(std::vector<float> &startP, std::vector<float> &finalP, std::vector<float> &direction);

void initWorld();
void initCamera();
void createObjects();

#endif
