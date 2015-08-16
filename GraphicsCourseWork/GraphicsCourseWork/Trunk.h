#pragma once
#include "SceneNode.h"
#include "Vector3.h"
#include <string>

#define VERTICES_PER_CIRCLE 20
#define TEXTURE_SCALE 100.0f

class Trunk :
	public Mesh
{
public:
	//use the texture int and bump map to intialize the texture for the trunk
	//the trunk is basically a cylinder, other than that,  the top circle don't actually have to be
	//parallel to the bottom one, but the normal of the top circle is needed, also, the center of the 
	//top circle is needed for the vertex coordinate calculating
	//other than that, we need the top diameter and root diameter
	//also the max height and the current height for the trunk
	Trunk(GLuint textureInt,GLuint bumpMap,Vector3 topCenterCoord = Vector3(0.0, 10.0, 0.0), Vector3 topNormal = Vector3(0.0, 1.0, 0.0), float topDiameter = 10.0, float rootDiameter = 10.0, float maxHeight = 20.0, float height = 0);
	virtual ~Trunk();

	//pass in the normal and the center coordinate and radius and the degree in circle, then get the coordinate of the vertex on the circle
	static Vector3 vertexCoordOfCircle(const Vector3& normal, const Vector3& centerCoord, const float& radius, const short& degree);

	//return the center coordinate for the top center
	Vector3 getTopCenterCoord() const { return topCenterCoord; };
	//return the normal of the top circle.
	Vector3 getTopNormal() const { return topNormal; };
	//return the top diameter for the trunk
	float getTopDiameter() const{ return topDiameter; };
	//return the root diameter for the trunk
	float getRootDiameter() const { return rootDiameter; };
	//return the max height of the trunk
	float getMaxHeight() const { return maxHeight; };
	//return the current height for the trunk
	float getHeight() const{ return height; };

protected:
	//the center coordinate of the top circle
	Vector3 topCenterCoord;
	//the normal of the top circle
	Vector3 topNormal;

	//the diameter of the top circle
	float topDiameter;
	//the root diameter of the bottom circle
	float rootDiameter;
	//the max height for the trunk
	float maxHeight;
	//the current height for the height
	float height;
};

