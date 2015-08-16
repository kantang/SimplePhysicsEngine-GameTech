#include "stdafx.h"
#include "Branch.h"


Branch::Branch(GLuint textureInt,GLuint bumpMap,float rootDiameter, float maxHeight, float height) : Mesh(GL_TRIANGLES)
{
	this->rootDiameter = rootDiameter;
	this->maxHeight = maxHeight;
	this->height = height;

	float topBottomRate = maxHeight / rootDiameter / rootDiameter;
	float textureHeight = height - ((int)height / 100) * 100;

	numVertices = VERTICES_PER_CIRCLE + 2;
	numIndices = VERTICES_PER_CIRCLE * 3;
	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new GLuint[numIndices];
	//initialize the vertices and their texture coordinate for the root circle
	for (int k = 0; k < VERTICES_PER_CIRCLE; ++k)
	{
		int degree = k * 360 / VERTICES_PER_CIRCLE;
		degree = degree % 360;
		vertices[k] = Trunk::vertexCoordOfCircle(Vector3(0, 1, 0), Vector3(0, 0, 0), rootDiameter / 2, degree);
		//colours[k] = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		textureCoords[k] = Vector2(((PI * rootDiameter / TEXTURE_SCALE) * degree / 360), (textureHeight) / TEXTURE_SCALE);
	}
	//initialize the vertex in the top and its texture coordinate
	vertices[VERTICES_PER_CIRCLE] = Vector3(0, maxHeight, 0);
	textureCoords[VERTICES_PER_CIRCLE] = Vector2(((PI * rootDiameter / TEXTURE_SCALE) / topBottomRate) / 2, (textureHeight + maxHeight) / TEXTURE_SCALE);
	//put a extra vertex in the end of the cylinder, to better maping the texture.
	vertices[VERTICES_PER_CIRCLE + 1] = vertices[0];
	textureCoords[VERTICES_PER_CIRCLE + 1] = Vector2(PI * rootDiameter / TEXTURE_SCALE, textureHeight / TEXTURE_SCALE);
	//initialize the index
	numIndices = 0;
	for (int z = 0; z < VERTICES_PER_CIRCLE - 1; ++z)
	{
		int a = z;
		int b = z + 1;
		int c = VERTICES_PER_CIRCLE;
		indices[numIndices++] = a;
		indices[numIndices++] = b;
		indices[numIndices++] = c;
	}
	//initialize the last part of the cylinder
	int a = VERTICES_PER_CIRCLE - 1;
	int b = VERTICES_PER_CIRCLE + 1;
	int c = VERTICES_PER_CIRCLE;
	indices[numIndices++] = a;
	indices[numIndices++] = b;
	indices[numIndices++] = c;

	GenerateNormals();
	GenerateTangents();
	SetTexture(textureInt);
	SetBumpMap(bumpMap);
	BufferData();
}

Branch::~Branch()
{
}

