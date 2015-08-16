#pragma once
#include "Mesh.h"
#include <string>
#include <iostream>
#include <fstream>

#define RAW_WIDTH 257
#define RAW_HEIGHT 257

#define HEIGHTMAP_X 16.0f
#define HEIGHTMAP_Z 16.0f
#define HEIGHTMAP_Y 1.25f
#define HEIGHTMAP_TEX_X 1.0f / 16.0f
#define HEIGHTMAP_TEX_Z 1.0f / 16.0f

class HeightMap :
	public Mesh
{
public:
	HeightMap(std::string name);
	~HeightMap(void) {};
	float getHeight(float x, float z)const{ return (vertices[(((int)(x / 16)) * RAW_WIDTH) + ((int)z / 16)]).y; };
};

