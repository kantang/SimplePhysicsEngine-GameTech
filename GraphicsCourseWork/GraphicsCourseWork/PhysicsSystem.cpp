#include "stdafx.h"
#include "PhysicsSystem.h"
#include "MyGame.h"

PhysicsSystem* PhysicsSystem::instance = 0;

PhysicsSystem::PhysicsSystem(void)	{
	fpsCounter = 0;
	collisionCounter = 0;
	useBroadPhaseCulling = false;
	currentMsec = 0.0f;

	score = 0.0;
}

PhysicsSystem::~PhysicsSystem(void)	{

}

void PhysicsSystem::Update(float msec) {
	//when update, first increment the fps counter.
	++fpsCounter;
	//set each frame the collison detection counter to zero first
	collisionDetectionCounter = 0;
	//set current msec = the msec got from outside
	currentMsec = msec;
	//check if using the Broad phase culling for collision detection.
	//if using it, use sort and sweep broad phase culling.
	if (useBroadPhaseCulling)
	{
		SortAndSweepCollisions();
	}
	else
	{
		BroadPhaseCollisions();
		NarrowPhaseCollisions();
	}
	
	//after the broad phase is created, do the collision detection in narrow phase.
	for (vector<PhysicsNode*>::iterator i = narrowPhaseNodes.begin(); i != narrowPhaseNodes.end(); ++i)
	{
		PhysicsNode* currentNode = (*i);
		if (currentNode->isNodeActive())
		{
			if (currentNode->GetCollisionCounter() > COLLISIONS_BEFORE_EXPLODE - 1)
			{
				((MyGame&)(MyGame::GetGameClass())).DestroySphereNode((SpherePhysicsNode*)currentNode);
			}
			else
				currentNode->Update(msec);

			if (dodgingEntity && currentNode->GetCollisionPrimitive()->GetType() == COLLISION_SPHERE)
			{
				Vector3 distance = dodgingEntity->GetPosition() - currentNode->GetPosition();
				float dis = sqrt(Vector3::Dot(distance, distance));
				if ( dis < dodgingEntity->GetRadius() + ((SpherePhysicsNode*)currentNode)->GetRadius() + 1000.0f)
				{
					dodgingEntity->SetLinearVelocity(dodgingEntity->GetLinearVelocity() + distance * 0.01f);
				}
			}
		}
	}
	if (dodgingEntity)
		dodgingEntity->Update(msec);
}

void PhysicsSystem::SortAndSweepCollisions()
{
	//sort all the nodes on x axis with the bounding radius
	narrowPhaseNodes.clear();
	narrowPhaseNodes = vector<PhysicsNode*>(allNodes);

	sort(narrowPhaseNodes.begin(), narrowPhaseNodes.end(), [](const PhysicsNode* a, const PhysicsNode* b){ return a->xBegin < b->xBegin; });
	for (vector<PhysicsNode*>::iterator i = narrowPhaseNodes.begin(); i != narrowPhaseNodes.end() - 1; ++i)
	{
		PhysicsNode* currentNode0 = (*i);
		if (currentNode0->GetCollisionPrimitive()->GetType() == UNKNOWN_TYPE)
			continue;
		for (vector<PhysicsNode*>::iterator k = i + 1; k != narrowPhaseNodes.end(); ++k)
		{
			PhysicsNode* currentNode1 = (*k);

			if (currentNode1->GetCollisionPrimitive()->GetType() == UNKNOWN_TYPE)
				continue;
			//if both node is not unknown type, then go on to sort and sweep.
			if (currentNode1->xBegin > currentNode0->xEnd)
				break;
			HandleCollision(currentNode0, currentNode1);
		}
	}
}

void PhysicsSystem::BroadPhaseCollisions() 
{
	//create a vector of nodes for narrow phase.
	narrowPhaseNodes.clear();
	narrowPhaseNodes = vector<PhysicsNode*>(allNodes);
}

void PhysicsSystem::NarrowPhaseCollisions() 
{
	//for each node in narrow phase collision detection, try to find out if it is colliding with other node.
	for (vector<PhysicsNode*>::iterator i = narrowPhaseNodes.begin(); i != narrowPhaseNodes.end() - 1; ++i)
	{
		PhysicsNode* currentNode0 = (*i);
		if (currentNode0->GetCollisionPrimitive()->GetType() == UNKNOWN_TYPE)
			continue;
		for (vector<PhysicsNode*>::iterator k = i + 1; k != narrowPhaseNodes.end(); ++k)
		{
			PhysicsNode* currentNode1 = (*k);
			
			if (currentNode1->GetCollisionPrimitive()->GetType() == UNKNOWN_TYPE)
				continue;
			//if both node is not unknown type, then do the collision detection within the node
			HandleCollision(currentNode0, currentNode1);
		}
	}
}

void PhysicsSystem::HandleCollision(PhysicsNode* currentNode0, PhysicsNode* currentNode1)
{
	//add a counter to collision detection.
	++collisionDetectionCounter;
	//if both node is not active then don't do collision detection between them since they can't collide while not moving
	if (!(currentNode0->isNodeActive() || currentNode1->isNodeActive()))
	{
		return;
	}
	CollisionPrimitiveTypes node0Type = currentNode0->GetCollisionPrimitive()->GetType();
	CollisionPrimitiveTypes node1Type = currentNode1->GetCollisionPrimitive()->GetType();
	CollisionData* collisionData = new CollisionData();
	switch (node0Type)
	{
	case COLLISION_SPHERE:
		//if it is sphere sphere, then do the sphere sphere collision detection
		if (node1Type == COLLISION_SPHERE)
		{
			if (SphereSphereCollision(((CollisionSphere&)(*(currentNode0->GetCollisionPrimitive()))), ((CollisionSphere&)(*(currentNode1->GetCollisionPrimitive()))), collisionData))
			{
				//first move out the object from the collision.
				Vector3 distance = collisionData->m_normal * collisionData->m_penetration;
				//the weight of each of the node moving is related to the mass of the node
				float weight0 = currentNode1->GetInverseMass() / (currentNode0->GetInverseMass() + currentNode1->GetInverseMass());
				float weight1 = currentNode0->GetInverseMass() / (currentNode0->GetInverseMass() + currentNode1->GetInverseMass());

				currentNode0->SetPosition(currentNode0->GetPosition() + distance * weight0);
				currentNode1->SetPosition(currentNode1->GetPosition() - distance * weight1);

				AddCollisionImpulse(*currentNode0, *currentNode1, *collisionData);
			}
		}
		//if it is sphere square, then do the sphere square collision detection
		else if (node1Type == COLLISION_SQUARE)
		{
			if (SpherePlaneCollision(((CollisionSphere&)(*(currentNode0->GetCollisionPrimitive()))), ((CollisionPlane&)(*(currentNode1->GetCollisionPrimitive()))), collisionData))
			{
				if (PointInSquare(*currentNode1, collisionData->m_point))
				{
					//first move out the object from the collision.
					Vector3 distance = collisionData->m_normal * collisionData->m_penetration;
					currentNode0->SetPosition(currentNode0->GetPosition() - distance);
					//then add impulse.
					AddCollisionImpulse(*currentNode0, *currentNode1, *collisionData);

				}
			}
		}
		else if (node1Type == COLLISION_HEIGHTMAP)
		{
			//if position is higher than the max height or lower than the minimum height, then early out
			if (currentNode0->GetPosition().y > ((HeightmapPhysicsNode*)currentNode1)->GetMaxHeight() ||
				currentNode0->GetPosition().y < ((HeightmapPhysicsNode*)currentNode1)->GetMinHeight())
				break;
			if (SphereHeightmapCollision(((CollisionSphere&)(*(currentNode0->GetCollisionPrimitive()))), ((CollisionHeightmap&)(*(currentNode1->GetCollisionPrimitive()))), collisionData))
			{
				//first move out the object from the collision.
				Vector3 distance = collisionData->m_normal * collisionData->m_penetration;
				currentNode0->SetPosition(currentNode0->GetPosition() - distance);
				//then add impulse.
				AddCollisionImpulse(*currentNode0, *currentNode1, *collisionData);
			}
		}
		else if (node1Type == COLLISION_CONE)
		{
			if (SphereConeCollision(((CollisionSphere&)(*(currentNode0->GetCollisionPrimitive()))), ((CollisionCone&)(*(currentNode1->GetCollisionPrimitive()))), collisionData))
			{
				//first move out the object from the collision.
				Vector3 distance = collisionData->m_normal * collisionData->m_penetration;
				currentNode0->SetPosition(currentNode0->GetPosition() - distance);
				//then add impulse.
				AddCollisionImpulse(*currentNode0, *currentNode1, *collisionData);
			}
		}
		break;
	case COLLISION_SQUARE:
		//if it is sphere square, then do the sphere square collision detection
		if (node1Type == COLLISION_SPHERE)
		{
			if (SpherePlaneCollision(((CollisionSphere&)(*(currentNode1->GetCollisionPrimitive()))), ((CollisionPlane&)(*(currentNode0->GetCollisionPrimitive()))), collisionData))
			{
				if (PointInSquare(*currentNode0, collisionData->m_point))
				{
					//first move out the object from the collision.
					Vector3 distance = collisionData->m_normal * collisionData->m_penetration;
					currentNode1->SetPosition(currentNode1->GetPosition() + distance);
					AddCollisionImpulse(*currentNode1, *currentNode0, *collisionData);
				}
			}
		}
		break;
	case COLLISION_CUBE:
		//if it is cube cube, then do the AABB collision detection
		if (node1Type == COLLISION_CUBE)
		{
			if (AABBCollision(((CollisionCube&)(*(currentNode1->GetCollisionPrimitive()))), ((CollisionCube&)(*(currentNode0->GetCollisionPrimitive())))))
			{
				AddCollisionImpulse(*currentNode0, *currentNode1, *collisionData);
			}
		}
		break;
	case COLLISION_HEIGHTMAP:
		if (node1Type == COLLISION_SPHERE)
		{
			//if position is higher than the max height or lower than the minimum height, then early out
			if (currentNode1->GetPosition().y > ((HeightmapPhysicsNode*)currentNode0)->GetMaxHeight() ||
				currentNode1->GetPosition().y < ((HeightmapPhysicsNode*)currentNode0)->GetMinHeight())
				break;
			if (SphereHeightmapCollision(((CollisionSphere&)(*(currentNode1->GetCollisionPrimitive()))), ((CollisionHeightmap&)(*(currentNode0->GetCollisionPrimitive()))), collisionData))
			{
				//first move out the object from the collision.
				Vector3 distance = collisionData->m_normal * collisionData->m_penetration;
				currentNode1->SetPosition(currentNode1->GetPosition() + distance);
				//then add impulse.
				AddCollisionImpulse(*currentNode1, *currentNode0, *collisionData);
			}
		}
		break;
	case COLLISION_CONE:
		if (node1Type == COLLISION_SPHERE)
		{
			if (SphereConeCollision(((CollisionSphere&)(*(currentNode1->GetCollisionPrimitive()))), ((CollisionCone&)(*(currentNode0->GetCollisionPrimitive()))), collisionData))
			{
				//first move out the object from the collision.
				Vector3 distance = collisionData->m_normal * collisionData->m_penetration;
				currentNode1->SetPosition(currentNode1->GetPosition() - distance);
				//then add impulse.
				AddCollisionImpulse(*currentNode1, *currentNode0, *collisionData);
			}
		}
		break;
	default:
		break;
	}
	delete collisionData;
}

void PhysicsSystem::AddNode(PhysicsNode* n) {
	allNodes.push_back(n);
}

void PhysicsSystem::RemoveNode(PhysicsNode* n) {
	for(vector<PhysicsNode*>::iterator i = allNodes.begin(); i != allNodes.end(); ++i) {
		if((*i) == n) {
			allNodes.erase(i);
			return;
		}
	}
}

bool PhysicsSystem::SphereSphereCollision(const CollisionSphere& s0, const CollisionSphere& s1, CollisionData *collisionData) const 
{
	//calculate the vector between two center points of the sphere
	Vector3 normal = s0.GetPosition() - s1.GetPosition();

	//use the normal to calculate the distance between the two center
	const float distSq = PhysicsSystem::LengthSquare(normal);

	//calculate the sum of the radius for two spheres
	const float sumRadius = (s0.m_radius + s1.m_radius);

	//if collide, then store the point
	if (distSq < sumRadius * sumRadius)
	{
		if (collisionData)
		{
			collisionData->m_penetration = sumRadius - sqrtf(distSq);
			normal.Normalise();
			collisionData->m_normal = normal;
			collisionData->m_point = s0.GetPosition() - normal * (s0.m_radius - collisionData->m_penetration * 0.5f);
		}
		return true;
	}
	return false;
}

bool PhysicsSystem::AABBCollision(const CollisionCube &cube0, const CollisionCube &cube1) const
{
	//Test along the x axis
	float dist = cube0.GetPosition().x - cube1.GetPosition().x;
	float sum = (cube0.halfSize.x + cube1.halfSize.x);
	if (dist <= sum)
	{
		//if test along the x axis is true, then test along the y axis
		dist = cube0.GetPosition().y - cube1.GetPosition().y;
		sum = cube0.halfSize.y + cube1.halfSize.y;
		if (dist <= sum)
		{
			//if test along the y axis is true, then test along the z axis
			dist = cube0.GetPosition().z - cube1.GetPosition().z;
			sum = cube0.halfSize.z + cube1.halfSize.z;
			if (dist <= sum)
				return true;
		}
	}
	return false;
}

bool PhysicsSystem::SpherePlaneCollision(const CollisionSphere &s, const CollisionPlane &p, CollisionData *collisionData) const
{
	//the speed direction must be the direction of the penetration.
	float separation = Vector3::Dot(s.GetPosition(), p.normal) - p.distance;
	if (abs(separation) > s.m_radius)
		return false;

	if (collisionData)
	{
		collisionData->m_penetration = s.m_radius - separation;
		collisionData->m_normal = p.normal;
		collisionData->m_point = s.GetPosition() - (p.normal * separation);
	}

	return true;
}

bool PhysicsSystem::SphereHeightmapCollision(const CollisionSphere &s, const CollisionHeightmap &h, CollisionData *collisionData) const
{
	//the speed direction must be the direction of the penetration.
	float height = h.GetHeight(s.GetPosition().x, s.GetPosition().z);
	if (height == FLT_MAX)
		return false;

	float separation = Vector3::Dot(s.GetPosition(), h.normal) - h.distance - height;

	if (abs(separation) > s.m_radius)
		return false;

	if (collisionData)
	{
		//cout << s.GetPosition() << endl;
		collisionData->m_penetration = s.m_radius - separation;
		collisionData->m_normal = h.normal;
		collisionData->m_point = s.GetPosition() - (h.normal * separation);
	}

	return true;
}

bool PhysicsSystem::SphereConeCollision(const CollisionSphere &s, const CollisionCone &c, CollisionData * collisionData) const
{
	Vector3 positionVec = s.GetPosition() - c.GetPosition();
	float length = Vector3::Dot(positionVec, c.normal);
	if (length > c.length || length < 0)
		return false;

	Vector3 point = c.GetPosition() + c.normal * length;

	Vector3 normal = s.GetPosition() - point;

	float radiusOnPoint = c.radius * length / c.length;

	float separation = sqrt(LengthSquare(normal));

	if (separation > s.m_radius + radiusOnPoint)
		return false;

	if (collisionData)
	{
		cout << "a collision between cone and sphere is made" << endl;
		collisionData->m_penetration = -(s.m_radius + radiusOnPoint - separation);
		normal.Normalise();
		collisionData->m_normal = normal;
		collisionData->m_point = s.GetPosition() - (normal * separation);
	}
	return true;
}


bool PhysicsSystem::PointInConvexPolygon(const Vector3 testPosition, Vector3 * convexShapePoints, int numPoints,const Vector3& normal) const 
{
	//check if our test point is inside our convex shape
	for (int i = 0; i < numPoints; ++i)
	{
		const int i0 = i;
		const int i1 = (i + 1) % numPoints;

		const Vector3& p0 = convexShapePoints[i0];
		const Vector3& p1 = convexShapePoints[i1];
		Vector3 p3 = Vector3(p0 - p1);
		p3.Normalise();
		const Vector3 n = Vector3::Cross(-normal, p3);

		const float d = Vector3::Dot(n, p0);
		const float s = d - Vector3::Dot(n, testPosition);

		if (s < 0.0f)
		{
			//failed, so skip rest of the tests
			return false;
		}
 	}
	return true;
}

float PhysicsSystem::LengthSquare(Vector3 vec)
{
	return Vector3::Dot(vec, vec);
}

bool PhysicsSystem::PointInSquare(const PhysicsNode& square, const Vector3& pointCoordinate)
{
	Vector3* vertices = new Vector3[4];
	square.GetTarget()->Update(0);
	const Matrix4& transform = square.GetTarget()->GetTransform();
	const Vector3& scale = square.GetTarget()->GetModelScale();
	vertices[0] = transform * (scale * Vector3(1.0f, 1.0f, 0.0f));
	vertices[1] = transform * (scale * Vector3(-1.0f, 1.0f, 0.0f));
	vertices[2] = transform * (scale * Vector3(-1.0f, -1.0f, 0.0f));
	vertices[3] = transform * (scale * Vector3(1.0f, -1.0f, 0.0f));
	                          
	return PointInConvexPolygon(pointCoordinate, vertices, 4,square.GetOrientation().GetNormal());
}

void PhysicsSystem::AddCollisionImpulse(PhysicsNode& p0, PhysicsNode& p1, CollisionData& data)
{
	//if both the mass of the collided object is infinite, then return.
	if (p0.GetInverseMass() + p1.GetInverseMass() == 0.0f)
		return;

	//caculate the distance between hitpoint and position
	Vector3 r0 = data.m_point - p0.GetPosition();
	Vector3 r1 = data.m_point - p1.GetPosition();

	//caculate the relative velocity for the two object
	Vector3 v0 = p0.GetLinearVelocity() + Vector3::Cross(p0.GetAngularVelocity(), r0);
	Vector3 v1 = p1.GetLinearVelocity() + Vector3::Cross(p1.GetAngularVelocity(), r1);

	/*v0 = v0 * p1.GetInverseMass() / (p1.GetInverseMass() + p0.GetInverseMass());
	v1 = v1 * p0.GetInverseMass() / (p1.GetInverseMass() + p0.GetInverseMass());*/

	Vector3 dv = v0 - v1;

	float relMov = -Vector3::Dot(dv, data.m_normal);
	if (relMov < -0.01f)
		return;

	//Normal Impulse
	{
		float e = ELASTICITY;
		float normDiv = (p0.GetInverseMass() + p1.GetInverseMass()) + Vector3::Dot(data.m_normal,
			Vector3::Cross(p0.GetInvInertia()*Vector3::Cross(r0, data.m_normal), r0) +
			Vector3::Cross(p1.GetInvInertia()*Vector3::Cross(r1, data.m_normal), r1));

		float jn = -1 * (1.0f + e) * Vector3::Dot(dv, data.m_normal) / normDiv;

		jn += (data.m_penetration * 0.1f);

		if (p0.GetCollisionPrimitive()->GetType() == COLLISION_SPHERE && p1.GetCollisionPrimitive()->GetType() == COLLISION_SPHERE)
			score += jn;

		//evaluate if the object should be at rest
		//if the impulse if equal to the negtive impulse of the gravity
		//and the velocity is relatively slow
		//then the object should be at rest.
		if (Vector3::Dot(p0.GetLinearVelocity(), p0.GetLinearVelocity()) < SPEED_REST_STATE_SQUARE && Vector3::Dot(p0.GetAngularVelocity(), p0.GetAngularVelocity()) < SPEED_REST_STATE_SQUARE)
		{
			Vector3 acceleration = data.m_normal * (p0.GetInverseMass() * jn / currentMsec);
			acceleration.y -= 0.001f * p0.GetGravityFactor();
//			cout << Vector3::Dot(acceleration, acceleration) << endl;
			if (Vector3::Dot(acceleration, acceleration) < ACCEL_REST_STATE_SQUARE)
			{
				p0.DeactivateNode();
				return;
			}
		}



		if (jn > 1.0f)
		{
			cout << "a collision with impulse of "<<jn << endl;
			++collisionCounter;
			if (p0.GetCollisionPrimitive()->GetType() == COLLISION_SPHERE && p1.GetCollisionPrimitive()->GetType() == COLLISION_SPHERE)
			{
				if (((SpherePhysicsNode&)p0).GetRadius() == ((SpherePhysicsNode&)p1).GetRadius() && ((SpherePhysicsNode&)p1).GetRadius() > 150.0f)
				{
					p0.CollisionCounterIncrement();
					p1.CollisionCounterIncrement();
				}
					
			}
		}

		

		Vector3 l0 = p0.GetLinearVelocity() + data.m_normal * (jn * p0.GetInverseMass());
		p0.SetLinearVelocity(l0);

		Vector3 a0 = p0.GetAngularVelocity() + p0.GetInvInertia() * Vector3::Cross(r0, data.m_normal * jn);
		p0.SetAngularVelocity(a0);

		Vector3 l1 = p1.GetLinearVelocity() - data.m_normal * (jn * p1.GetInverseMass());
		p1.SetLinearVelocity(l1);
		Vector3 a1 = p1.GetAngularVelocity() - p1.GetInvInertia() * Vector3::Cross(r1, data.m_normal * jn);
		p1.SetAngularVelocity(a1);
	}

	//TANGENT Impulse Code
	{
		Vector3 tangent = dv - data.m_normal * Vector3::Dot(dv, data.m_normal);
		tangent.Normalise();
		float tangDiv = (p0.GetInverseMass() + p1.GetInverseMass()) + Vector3::Dot(tangent,
			Vector3::Cross(p0.GetInvInertia() * Vector3::Cross(r0, tangent), r0) +
			Vector3::Cross(p1.GetInvInertia() * Vector3::Cross(r1, tangent), r1));

		float jt = -1 * Vector3::Dot(dv, tangent) / tangDiv;
		Vector3 l0 = p0.GetLinearVelocity() + tangent * (jt * p0.GetInverseMass());
		p0.SetLinearVelocity(l0);
		Vector3 a0 = p0.GetAngularVelocity() + p0.GetInvInertia() * Vector3::Cross(r0, tangent * jt);
		p0.SetAngularVelocity(a0);

		Vector3 l1 = p1.GetLinearVelocity() - tangent * (jt * p1.GetInverseMass());
		p1.SetLinearVelocity(l1);

		Vector3 a1 = p1.GetAngularVelocity() - p1.GetInvInertia() * Vector3::Cross(r1, tangent * jt);
		p1.SetAngularVelocity(a1);
	}

	//after that set all the node to active
	p0.ActivateNode();
	p1.ActivateNode();

}