#pragma once
#include "OGLRenderer.h"
#include <vector>

using namespace std;

enum MeshBuffer
{
	VERTEX_BUFFER, COLOUR_BUFFER, TEXTURE_BUFFER, NORMAL_BUFFER, TANGENT_BUFFER, INDEX_BUFFER, MAX_BUFFER
};

#define BYTES_PER_FLOAT 4;

class Mesh
{
public:
	Mesh(int gl_type = GL_TRIANGLES);
	~Mesh(void);

	virtual void Draw();
	static Mesh* GenerateTriangle(vector<Vector3> triangleVertices, vector<Vector4> colors, vector<Vector2> txtrCoords, vector<Vector3>normals, vector<Vector3>tangents);
	static Mesh* GenerateQuad(vector<Vector3> quadVertices, vector<Vector4> colors, vector<Vector2> txtrCoords, vector<Vector3>normals, vector<Vector3>tangents);
	static Mesh* GenerateTriangle();
	static Mesh* GenerateQuad();

	void* mapBuffer(int bufferID);
	void unmapBuffer();
	
	void SetBumpMap(GLuint tex) { bumpTexture = tex; };
	GLuint GetBumpMap() { return bumpTexture; };

	void SetTexture(GLuint tex) { texture = tex; };
	GLuint GetTexture(){ return texture; };

	unsigned int getMemory();

	GLuint GetNumVertices(){ return numVertices; };
	Vector3* GetVertices() const { return vertices; };

protected:
	void BufferData();
	void GenerateNormals();
	void GenerateTangents();
	Vector3 GenerateTangent(const Vector3 &a, const Vector3 &b, const Vector3 &c, const Vector2 &ta, const Vector2 &tb, const Vector2 tc);

	Vector3* tangents;
	GLuint bumpTexture;

	GLuint numIndices;
	unsigned int* indices;
	GLuint arrayObject;
	GLuint bufferObject[MAX_BUFFER];
	GLuint numVertices;
	GLuint texture;
	GLuint type;

	Vector2* textureCoords;
	Vector3* vertices;
	Vector3* normals;
	Vector4* colours;
};

