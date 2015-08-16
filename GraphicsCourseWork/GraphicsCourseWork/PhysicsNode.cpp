#include "stdafx.h"
#include "PhysicsNode.h"

PhysicsNode::PhysicsNode(void)	{
	target = NULL;
	isActive = false;
	m_force.ToZero();
	m_force_position.ToZero();
	m_centroid_position.ToZero();
	m_gravity_factor = NON_GRAVITY;
	m_invMass = 0.0f;
	collisionPrimitive = NULL;
	collisionCounter = 0;
	dumpingFactor = DUMPING_FACTOR;
}

PhysicsNode::PhysicsNode(Quaternion orientation, Vector3 position) {
	target = NULL;
	m_orientation	= orientation;
	m_position		= position;
	m_centroid_position.ToZero();
	isActive = false;
	m_gravity_factor = NON_GRAVITY;
	m_invMass = 0.0f;
	collisionPrimitive = NULL;
	dumpingFactor = DUMPING_FACTOR;

}

PhysicsNode::~PhysicsNode(void)	{

}

//You will perform your per-object physics integration, here!
//I've added in a bit that will set the transform of the
//graphical representation of this object, too.
void	PhysicsNode::Update(float msec) {
	//initialize the collision primitive to basic type if not initalized
	if (collisionPrimitive == NULL)
	{
		collisionPrimitive = new CollisionPrimitive(&m_position);
	}
	if (m_invMass != 0)
	{
		m_gravity = m_gravity_factor / m_invMass;
		//FUN GOES HERE
		float dt = msec * 0.001f;
		//Updating the linear related factor
		//get the total force on the objects' centroid
		Vector3 totalForce = Vector3(0, -m_gravity, 0) + ForceOnCentroid();
		//Semi Implicit Euler Integration
		m_linearVelocity = m_linearVelocity + totalForce * m_invMass * dt;
		//apply dumping factor;
		m_linearVelocity = m_linearVelocity * dumpingFactor;

		//if lower than minimum velocity, then discard.
		if (Vector3::Dot(m_linearVelocity, m_linearVelocity) < MINIMUM_VELOCITY_SQUARE)
			m_linearVelocity.ToZero();
		//calculate position
		m_position = m_position + m_linearVelocity * msec;

		//Updating the angular related factor
		BuildTorque();
		Vector3 angularAcceleration = m_invInertia * m_torque;
		//calculate the angular velocity
		m_angularVelocity = m_angularVelocity + angularAcceleration * dt;
		//apply dumping factor
		m_angularVelocity = m_angularVelocity * dumpingFactor;
		//calculate the new orientation
		m_orientation.Plus((m_orientation * (m_angularVelocity * (msec * 0.5f))));
		//normalise it because the approximation.
		m_orientation.Normalise();

		//FUN ENDS >.<
	}
	else
	{
		m_torque.ToZero();
	}
	if(target) {
		target->SetRotation(BuildTransform());
	}

	xBegin = m_position.x - target->GetBoundingRadius();
	xEnd   = m_position.x + target->GetBoundingRadius();
}

/*
This function simply turns the orientation and position
of our physics node into a transformation matrix, suitable
for plugging into our Renderer!

It is cleaner to work with matrices when it comes to rendering,
as it is what shaders expect, and allow us to keep all of our
transforms together in a single construct. But when it comes to
physics processing and 'game-side' logic, it is much neater to
have seperate orientations and positions.

*/
Matrix4		PhysicsNode::BuildTransform() {
	Matrix4 m = m_orientation.ToMatrix();

	m.SetPositionVector(m_position);

	return m;
}

void PhysicsNode::BuildTorque()
{
	Vector3 centroidVec = m_centroid_position - m_force_position;
	m_torque = Vector3::Cross(centroidVec, m_force);
}

Vector3 PhysicsNode::ForceOnCentroid()
{
	if (m_force_position == m_centroid_position)
	{
		return m_force;
	}
	else
	{
		Vector3 centroidVec = m_centroid_position - m_force_position;
		float newForce = Vector3::Dot(centroidVec, m_force);
		centroidVec.Normalise(); 
		return (centroidVec * newForce);
	}
}

void PhysicsNode::ActivateNode()
{
	isActive = true;
	if (target)
	{
		target->SetColour(initialColour);
	}
}

void PhysicsNode::DeactivateNode()
{
	isActive = false;
	if (target)
	{
		target->SetColour(Vector4(0.0f,0.0f,0.0f,1.0f));
	}
}