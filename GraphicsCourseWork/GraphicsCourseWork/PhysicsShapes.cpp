#include "stdafx.h"
#include "PhysicsShapes.h"

void CuboidPhysicsNode::BuildInertiaMatrix()
{
	const float constant = m_invMass * ONE_TWELVE;
	//TODO: test if this is right
	const float lSquare = size.x * size.x;
	const float hSquare = size.y * size.y;
	const float wSquare = size.z * size.z;

	m_invInertia(0, 0) = constant * (hSquare + wSquare);
	m_invInertia(1, 1) = constant * (lSquare + wSquare);
	m_invInertia(2, 2) = constant * (hSquare + lSquare);

	InverseInertia();
}

void CuboidPhysicsNode::InverseInertia()
{
	m_invInertia(0, 0) = 1 / m_invInertia(0, 0);
	m_invInertia(1, 1) = 1 / m_invInertia(1, 1);
	m_invInertia(2, 2) = 1 / m_invInertia(2, 2);
}

void SpherePhysicsNode::BuildInertiaMatrix()
{
	const float i = TWO_FIFTHS * m_invMass * radius * radius;

	m_invInertia(0, 0) = i;
	m_invInertia(1, 1) = i;
	m_invInertia(2, 2) = i;

	InverseInertia();
}

void SpherePhysicsNode::InverseInertia()
{
	float inverseI = 1 / m_invInertia(0, 0);

	m_invInertia(0, 0) = inverseI;
	m_invInertia(1, 1) = inverseI;
	m_invInertia(2, 2) = inverseI;
}

//not finished yet, to avoid flipping the square, just set inertia matrix to zero.
void SquarePhysicsNode::BuildInertiaMatrix()
{
	m_invInertia.ToZero();
}

void HeightmapPhysicsNode::BuildInertiaMatrix()
{
	m_invInertia.ToZero();
}

void HeightmapPhysicsNode::InitializeHeightmapCoord()
{
	if (!target)
		return;

	

	heightMapCoordinatesMap.clear();

	target->Update(0.0f);
	Mesh* mesh = target->GetMesh();

	Vector3 * vertices = mesh->GetVertices();
	unsigned int numVertices = mesh->GetNumVertices();

	minHeight = vertices->y;
	maxHeight = vertices->y;
	//create a vector with all the information of the vertices' coordinate
	//this coordinate has already mutiplied the scenenode's world transformation
	{
		vector<Vector3> totalVector;

		Vector3 translate = target->GetWorldTransform() * target->GetModelScale();

		for (unsigned int i = 0; i < numVertices; i++)
		{
			Vector3 coordinate = translate * (vertices[i]);

			//get the max and min height of the coordinate to early out the collision detection
			maxHeight = maxHeight > coordinate.y ? maxHeight : coordinate.y;
			minHeight = minHeight < coordinate.y ? minHeight : coordinate.y;

			totalVector.push_back(coordinate);
		}
		//sort the vector on the x axis
		sort(totalVector.begin(), totalVector.end(), [](const Vector3& a, const Vector3& b){ return (a).x < (b).x; });

		float x = 0.0f;

		map<float,float> xMap;

		x = totalVector[0].x;

		for (unsigned int i = 0; i < numVertices; i++)
		{
			if (x != totalVector[i].x)
			{
				heightMapCoordinatesMap.insert(std::pair<float, map<float, float>>(x, xMap));
				xMap.clear();
				x = totalVector[i].x;
			}
			xMap.insert(std::pair<float,float>(totalVector[i].z,totalVector[i].y));
		}
		heightMapCoordinatesMap.insert(std::pair<float, map<float, float>>(x, xMap));
	}
}

void ConePhysicsNode::BuildInertiaMatrix()
{
	m_invInertia.ToZero();
}