#pragma once
#include "Mesh.h"
#include "Trunk.h"
#define VERTICES_PER_CIRCLE 20
#ifndef TEXTURE_SCALE
#define TEXTURE_SCALE 100.0f
#endif
class Branch :
	public Mesh
{
public:
	//create the branch using the attribute below
	//load the texture once to speed up
	//just a cone that shoots straight up
	Branch(GLuint textureInt,GLuint bumpMap,float rootDiameter = 10.0, float maxHeight = 20.0, float height = 0);
	virtual ~Branch();

	//get the diameter of the root circle
	float getRootDiameter() const { return rootDiameter; };
	//get the max height for the branch, usually means the height when the scale is Vector3(1,1,1)
	float getMaxHeight() const { return maxHeight; };
	//get the height of the branch, use this to better mapping the texture.
	float getHeight() const{ return height; };

protected:
	//the root diameter of the circle
	float rootDiameter;
	//the total height of the branch
	float maxHeight;
	//the height used to calculate the texture coordinate
	float height;
};

