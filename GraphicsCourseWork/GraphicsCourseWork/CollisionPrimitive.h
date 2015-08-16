#pragma once
#include "..\..\nclgl\Vector3.h"
#include "Tree.h"
#include <map>


enum CollisionPrimitiveTypes
{
	UNKNOWN_TYPE,COLLISION_SPHERE,COLLISION_SQUARE,COLLISION_CUBE,COLLISION_HEIGHTMAP,COLLISION_CONE
};

using namespace std;

//the super class for all the collision primitive
class CollisionPrimitive
{
public:
	CollisionPrimitive(Vector3* p)
	{
		m_pos = p; 
		type = UNKNOWN_TYPE; 
	};

	virtual ~CollisionPrimitive(){};

	Vector3 GetPosition() const{ return *m_pos; };

	CollisionPrimitiveTypes GetType(){ return type; };

protected:
	Vector3* m_pos;
	CollisionPrimitiveTypes type;
};

class CollisionSphere : public CollisionPrimitive
{
public:
	CollisionSphere(Vector3* p, float r) :
		CollisionPrimitive(p)
	{
		m_radius = r;
		type = COLLISION_SPHERE;
	}

	float m_radius;
};

class CollisionCube : public CollisionPrimitive
{
public:
	CollisionCube(Vector3* p, const Vector3& halfDim) :
		CollisionPrimitive(p)
	{
		halfSize = halfDim;
		type = COLLISION_CUBE;
	}

	Vector3 halfSize;
};

class CollisionPlane : public CollisionPrimitive
{
public:
	CollisionPlane(Vector3* p, const Vector3& n, float dis) :
		CollisionPrimitive(p)
	{
		normal = n;
		distance = dis;
		type = COLLISION_SQUARE;
	}

	Vector3 normal;
	float distance;
};

class CollisionHeightmap : public CollisionPrimitive
{
public:
	CollisionHeightmap(Vector3* p, const Vector3& n, float dis, map<float, map<float, float>>* cMap) :
		CollisionPrimitive(p)
	{
		normal = n;
		distance = dis;
		type = COLLISION_HEIGHTMAP;
		coordMap = cMap;
	}

	Vector3 normal;
	float distance;
	map<float, map<float, float>>* coordMap;

	float GetHeight(float x, float z) const {
		
		map<float, map<float, float>>::iterator b = coordMap->begin();
		map<float, map<float, float>>::iterator e = coordMap->end();
		--e;

		float min = b->first;
		float max = e->first;

		if (x < min || x > max ||
			z < min || z > max)
			return FLT_MAX;
		float dis = (max - min) / (coordMap->size() - 1);
		float x1 = (float)((int)x - ((int)x % (int)dis));
		map<float, float>& mapX1 = (coordMap->find(x1))->second;
		float x2 = dis + x1;
		map<float, float>& mapX2 = (coordMap->find(x2))->second;
		//find the 4 closest points to the given coordinate
		float z0[4];
		float y[4];


		for (map<float, float>::iterator i = mapX1.begin(); i != mapX1.end(); i++)
		{
			if (z <= i->first)
				continue;
			else
			{
				if (i == mapX1.begin())
					++i;
				z0[0] = i->first;
				y[0] = i->second;
				--i;
				z0[1] = i->first;
				y[1] = i->second;
				break;
			}
		}

		for (map<float, float>::iterator i = mapX2.begin(); i != mapX2.end(); i++)
		{
			if (z <= i->first)
				continue;
			else
			{
				if (i == mapX2.begin())
					++i;
				z0[2] = i->first;
				y[2] = i->second;
				--i;
				z0[3] = i->first;
				y[3] = i->second;
				break;
			}
		}
		//calculate the 4 weight
	
		float weight[4];
		weight[0] = sqrt((x1 - x) * (x1 - x) + (z0[0] - z) * (z0[0] - z));
		weight[1] = sqrt((x1 - x) * (x1 - x) + (z0[1] - z) * (z0[1] - z));
		weight[2] = sqrt((x2 - x) * (x2 - x) + (z0[2] - z) * (z0[2] - z));
		weight[3] = sqrt((x2 - x) * (x2 - x) + (z0[3] - z) * (z0[3] - z));
		
		float weightSum = weight[0] + weight[1] + weight[2] + weight[3];
		weight[0] = weight[0] / weightSum;
		weight[1] =	weight[1] / weightSum;
		weight[2] =	weight[2] / weightSum;
		weight[3] =	weight[3] / weightSum;

		return y[0] * weight[0] +
			   y[1] * weight[1] +
			   y[2] * weight[2] +
			   y[3] * weight[3];
	};
};

class CollisionCone : public CollisionPrimitive
{
public:
	CollisionCone(Vector3* p, const Vector3& n, float len, float rad) :
		CollisionPrimitive(p)
	{
		normal = n;
		length = len;
		radius = rad;
		type = COLLISION_CONE;
	}

	Vector3 normal;
	float length;
	float radius;
};
