#include "stdafx.h"
#include "Trunk.h"


Trunk::Trunk(GLuint textureInt,GLuint bumpMap, Vector3 topCenterCoord, Vector3 topNormal, float topDiameter, float rootDiameter, float maxHeight, float height) : Mesh(GL_TRIANGLES)
{
	this->topNormal = topNormal;
	this->topDiameter = topDiameter;
	this->rootDiameter = rootDiameter;
	this->topCenterCoord = topCenterCoord;
	this->maxHeight = maxHeight;
	this->height = height;

	float topBottomRate = topDiameter / rootDiameter;
	float textureHeight = height - ((int)height / 100) * 100;

	numVertices = VERTICES_PER_CIRCLE * 2 + 2;
	numIndices = VERTICES_PER_CIRCLE * 3 * 2 + 2;
	vertices = new Vector3[numVertices];
	//colours = new Vector4[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new GLuint[numIndices];
	//initialize the vertices and their texture coordinate for the root circle
	for (int k = 0; k < VERTICES_PER_CIRCLE; ++k)
	{
		int degree = k * 360 / VERTICES_PER_CIRCLE;
		degree = degree % 360;
		vertices[k] = Trunk::vertexCoordOfCircle(Vector3(0, 1, 0), Vector3(0, 0, 0), rootDiameter / 2, degree);
		//colours[k] = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		textureCoords[k] = Vector2(((PI * rootDiameter / TEXTURE_SCALE) * degree / 360), textureHeight / TEXTURE_SCALE);
	}
	//initialize the vertices and their texture coordinate for the top circle
	for (int k = VERTICES_PER_CIRCLE; k < VERTICES_PER_CIRCLE * 2 + 1; ++k)
	{
		int degree = k * 360 / VERTICES_PER_CIRCLE;
		degree = degree % 360;
		vertices[k] = Trunk::vertexCoordOfCircle(topNormal, topCenterCoord, topDiameter / 2, degree);
		//colours[k] = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		textureCoords[k] = Vector2((float)(((PI * topDiameter) / TEXTURE_SCALE / topBottomRate) * degree / 360), (float)((topCenterCoord.y + textureHeight) / TEXTURE_SCALE));
	}
	//put two extra vertices in the end of the cylinder, to better maping the texture.
	vertices[VERTICES_PER_CIRCLE * 2] = vertices[0];
	vertices[VERTICES_PER_CIRCLE * 2 + 1] = vertices[VERTICES_PER_CIRCLE];
	textureCoords[VERTICES_PER_CIRCLE * 2] = Vector2((float)(PI * rootDiameter / TEXTURE_SCALE), textureHeight / TEXTURE_SCALE);
	textureCoords[VERTICES_PER_CIRCLE * 2 + 1] = Vector2((float)(PI * topDiameter / TEXTURE_SCALE / topBottomRate), ((topCenterCoord.y + textureHeight) / TEXTURE_SCALE));
	//initialize the index
	numIndices = 0;
	for (int z = 0; z < VERTICES_PER_CIRCLE - 1; ++z)
	{
		int a = z;
		int b = VERTICES_PER_CIRCLE + z;
		int c = VERTICES_PER_CIRCLE + z + 1;
		int d = z + 1;

		indices[numIndices++] = a;
		indices[numIndices++] = d;
		indices[numIndices++] = b;

		indices[numIndices++] = c;
		indices[numIndices++] = b;
		indices[numIndices++] = d;
	}
	//initialize the last part of the cylinder
	int a = VERTICES_PER_CIRCLE - 1;
	int b = VERTICES_PER_CIRCLE + VERTICES_PER_CIRCLE - 1;
	int c = VERTICES_PER_CIRCLE * 2 + 1;
	int d = VERTICES_PER_CIRCLE * 2;
	indices[numIndices++] = a;
	indices[numIndices++] = d;
	indices[numIndices++] = b;

	indices[numIndices++] = c;
	indices[numIndices++] = b;
	indices[numIndices++] = d;

	GenerateNormals();
	GenerateTangents();
	SetTexture(textureInt);
	SetBumpMap(bumpMap);

	BufferData();
}

Trunk::~Trunk()
{
}


Vector3 Trunk::vertexCoordOfCircle(const Vector3& normal, const Vector3& centerCoord, const float& radius, const short& degree)
{
	Vector3 uV = Vector3(normal.y, normal.x, 0);
	Vector3 vV = Vector3::Cross(normal, uV);
	uV.Normalise();
	vV.Normalise();
	float rad = PI * degree / 180;
	auto circle = [&](float center, float u, float v){ return center + radius * ((u * cos(rad)) + (v * sin(rad))); };

	return Vector3(circle(centerCoord.x, uV.x, vV.x), circle(centerCoord.y, uV.y, vV.y), circle(centerCoord.z, uV.z, vV.z));
}