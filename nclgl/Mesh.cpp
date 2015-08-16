#include "Mesh.h"


Mesh::Mesh(int gl_type)
{
	for (int i = 0; i < MAX_BUFFER; ++i)
		bufferObject[i] = 0;

	glGenVertexArrays(1, &arrayObject);

	numVertices = 0;
	texture = 0;
	textureCoords = NULL;
	vertices = NULL;
	indices = NULL;
	normals = NULL;
	tangents = NULL;
	bumpTexture = 0;
	numIndices = 0;
	colours = NULL;
	type = gl_type;
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &arrayObject);
	glDeleteBuffers(MAX_BUFFER, bufferObject);
	glDeleteTextures(1, &texture);
	glDeleteTextures(1, &bumpTexture);
	delete[] textureCoords;
	delete[] vertices;
	delete[] tangents;
	delete[] indices;
	delete[] colours;
	delete[] normals;
	
}

Mesh* Mesh::GenerateTriangle(vector<Vector3> triangleVertices, vector<Vector4> colors, vector<Vector2> txtrCoords, vector<Vector3>normals, vector<Vector3>tangents)
{
	Mesh*m = new Mesh(GL_TRIANGLES);
	m->numVertices = 3;
	m->vertices = new Vector3[m->numVertices];
	m->vertices[0] = triangleVertices[0];
	m->vertices[1] = triangleVertices[1];
	m->vertices[2] = triangleVertices[2];

	m->textureCoords = new Vector2[m->numVertices];
	m->textureCoords[0] = txtrCoords[0];
	m->textureCoords[1] = txtrCoords[1];
	m->textureCoords[2] = txtrCoords[2];

	m->colours = new Vector4[m->numVertices];
	m->colours[0] = colors[0];
	m->colours[1] = colors[1];
	m->colours[2] = colors[2];

	m->normals = new Vector3[m->numVertices];
	m->normals[0] = normals[0];
	m->normals[1] = normals[1];
	m->normals[2] = normals[2];

	m->tangents = new Vector3[m->numVertices];
	m->tangents[0] = tangents[0];
	m->tangents[1] = tangents[1];
	m->tangents[2] = tangents[2];

	m->BufferData();
	return m;
}

Mesh* Mesh::GenerateQuad(vector<Vector3> quadVertices, vector<Vector4> colors, vector<Vector2> txtrCoords, vector<Vector3>normals, vector<Vector3>tangents)
{
	Mesh*m = new Mesh(GL_TRIANGLE_STRIP);
	m->numVertices = 4;
	
	m->vertices = new Vector3[m->numVertices];
	m->vertices[0] = quadVertices[0];
	m->vertices[1] = quadVertices[1];
	m->vertices[2] = quadVertices[2];
	m->vertices[3] = quadVertices[3];

	m->textureCoords = new Vector2[m->numVertices];
	m->textureCoords[0] = txtrCoords[0];
	m->textureCoords[1] = txtrCoords[1];
	m->textureCoords[2] = txtrCoords[2];
	m->textureCoords[3] = txtrCoords[3];

	m->colours = new Vector4[m->numVertices];
	m->colours[0] = colors[0];
	m->colours[1] = colors[1];
	m->colours[2] = colors[2];
	m->colours[3] = colors[3];

	m->normals = new Vector3[m->numVertices];
	m->normals[0] = normals[0];
	m->normals[1] = normals[1];
	m->normals[2] = normals[2];
	m->normals[3] = normals[3];

	m->tangents = new Vector3[m->numVertices];
	m->tangents[0] = tangents[0];
	m->tangents[1] = tangents[1];
	m->tangents[2] = tangents[2];
	m->tangents[3] = tangents[3];

	m->BufferData();
	return m;
}

Mesh* Mesh::GenerateTriangle()
{
	vector<Vector3> default_TV = { { 0.0f, 0.5f, 0.0f }, { 0.5f, -0.5f, 0.0f }, { -0.5f, -0.5f, 0.0f } };
	vector<Vector4> default_TC = { { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } };
	vector<Vector2> default_TCC = { { 0.5f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
	vector<Vector3> default_TNM = { { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f } };
	vector<Vector3> default_TTG = { { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } };
	return Mesh::GenerateTriangle(default_TV, default_TC, default_TCC,default_TNM, default_TTG);
}

Mesh* Mesh::GenerateQuad()
{
	vector<Vector3> default_QV = { { -1.0f, -1.0f, 0.0f }, { -1.0f, 1.0f, 0.0f }, { 1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } };
	vector<Vector4> default_QC = { { 1, 1, 1, 1 }, { 1, 1, 1, 1 }, { 1, 1, 1, 1 }, { 1, 1, 1, 1 } };
	vector<Vector2> default_QCC = { { 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f } };
	vector<Vector3> default_QNM = { { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f } };
	vector<Vector3> default_QTG = { { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } };


	return Mesh::GenerateQuad(default_QV, default_QC, default_QCC,default_QNM,default_QTG);
}

void Mesh::BufferData()
{
	//bind the vertex array with the name of array object.
	glBindVertexArray(arrayObject);
	//generate a new vertex buffer object
	glGenBuffers(1, &bufferObject[VERTEX_BUFFER]);
	//bind it, so all the Vertex Buffer functions are performed on our Vertex Buffer, and tye VBOs into a single VAO
	glBindBuffer(GL_ARRAY_BUFFER, bufferObject[VERTEX_BUFFER]);
	//copy data into graphics memory GL_STATIC_DRAW means constant
	glBufferData(GL_ARRAY_BUFFER, numVertices*sizeof(Vector3), vertices, GL_STATIC_DRAW);
	//has 3 float per vertex
	glVertexAttribPointer(VERTEX_BUFFER, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//enable the vertex buffer
	glEnableVertexAttribArray(VERTEX_BUFFER);

	if (textureCoords)
	{
		glGenBuffers(1, &bufferObject[TEXTURE_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[TEXTURE_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, numVertices*sizeof(Vector2), textureCoords, GL_STATIC_DRAW);
		glVertexAttribPointer(TEXTURE_BUFFER, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(TEXTURE_BUFFER);
	}

	if (indices)
	{
		glGenBuffers(1, &bufferObject[INDEX_BUFFER]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObject[INDEX_BUFFER]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices*sizeof(GLuint), indices, GL_STATIC_DRAW);
	}

	if (colours)
	{
		glGenBuffers(1, &bufferObject[COLOUR_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[COLOUR_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, numVertices*sizeof(Vector4), colours, GL_STATIC_DRAW);
		glVertexAttribPointer(COLOUR_BUFFER, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(COLOUR_BUFFER);
	}
	if (normals)
	{
		glGenBuffers(1, &bufferObject[NORMAL_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[NORMAL_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, numVertices*sizeof(Vector3), normals, GL_STATIC_DRAW);
		glVertexAttribPointer(NORMAL_BUFFER, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(NORMAL_BUFFER);
	}
	if (tangents)
	{
		glGenBuffers(1, &bufferObject[TANGENT_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[TANGENT_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, numVertices*sizeof(Vector3), tangents, GL_STATIC_DRAW);
		glVertexAttribPointer(TANGENT_BUFFER, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(TANGENT_BUFFER);
	}
	glBindVertexArray(0);
}

void Mesh::GenerateNormals()
{
	if (!normals)
	{
		normals = new Vector3[numVertices];
	}
	for (GLuint i = 0; i < numVertices; ++i)
		normals[i] = Vector3();

	if (indices)
	{
		for (GLuint i = 0; i < numIndices; i += 3)
		{
			unsigned int a = indices[i];
			unsigned int b = indices[i + 1];
			unsigned int c = indices[i + 2];

			Vector3 normal = Vector3::Cross((vertices[b] - vertices[a]), (vertices[c] - vertices[a]));
			
			normals[a] += normal;
			normals[b] += normal;
			normals[c] += normal;
		}
	}
	else
	{
		for (GLuint i = 0; i < numVertices; i += 3)
		{
			Vector3 &a = vertices[i];
			Vector3 &b = vertices[i + 1];
			Vector3 &c = vertices[i + 2];

			Vector3 normal = Vector3::Cross(b - a, c - a);

			normals[i] = normal;
			normals[i + 1] = normal;
			normals[i + 2] = normal;
		}
	}

	for (GLuint i = 0; i < numVertices; ++i)
	{
		normals[i].Normalise();
	}
}

void Mesh::GenerateTangents()
{
	if (!tangents)
		tangents = new Vector3[numVertices];
	for (GLuint i = 0; i < numVertices; ++i)
		tangents[i] = Vector3();

	if (indices)
	{
		for (GLuint i = 0; i < numIndices; i += 3)
		{
			int a = indices[i];
			int b = indices[i + 1];
			int c = indices[i + 2];

			Vector3 tangent = GenerateTangent(vertices[a], vertices[b], vertices[c], textureCoords[a], textureCoords[b], textureCoords[c]);

			tangents[a] += tangent;
			tangents[b] += tangent;
			tangents[c] += tangent;
		}
	}
	else
	{
		for (GLuint i = 0; i < numVertices; i += 3)
		{
			Vector3 tangent = GenerateTangent(vertices[i], vertices[i + 1], vertices[i + 2], textureCoords[i], textureCoords[i + 1], textureCoords[i + 2]);

			tangents[i] += tangent;
			tangents[i + 1] += tangent;
			tangents[i + 2] += tangent;
		}
	}
	for (GLuint i = 0; i < numVertices; ++i)
		tangents[i].Normalise();
}

Vector3 Mesh::GenerateTangent(const Vector3 &a, const Vector3 &b, const Vector3 &c, const Vector2 &ta, const Vector2 &tb, const Vector2 tc)
{
	Vector2 coord1 = tb - ta;
	Vector2 coord2 = tc - ta;

	Vector3 vertex1 = b - a;
	Vector3 vertex2 = c - a;

	Vector3 axis = Vector3(vertex1 * coord2.y - vertex2 * coord1.y);

	float factor = 1.0f / (coord1.x * coord2.y - coord2.x * coord1.y);

	return axis * factor;
}

void Mesh::Draw()
{


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bumpTexture);

	glBindVertexArray(arrayObject);
	if (bufferObject[INDEX_BUFFER])
		glDrawElements(type, numIndices, GL_UNSIGNED_INT, 0);
	else
		glDrawArrays(type, 0, numVertices);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);

}

void* Mesh::mapBuffer(int bufferID)
{
	glBindBuffer(GL_ARRAY_BUFFER, bufferObject[bufferID]);
	return glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
}

void Mesh::unmapBuffer()
{
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

unsigned int Mesh::getMemory()
{
	int memoryCount = 0;
	memoryCount += numVertices * 3 * BYTES_PER_FLOAT;
	if (textureCoords)
		memoryCount += numVertices * 2 * BYTES_PER_FLOAT;
	if (indices)
		memoryCount += numIndices * 3 * BYTES_PER_FLOAT;
	if (colours)
		memoryCount += numVertices * 4 * BYTES_PER_FLOAT;
	if (normals)
		memoryCount += numVertices * 3 * BYTES_PER_FLOAT;
	if (tangents)
		memoryCount += numVertices * 3 * BYTES_PER_FLOAT;

	return memoryCount;
}
