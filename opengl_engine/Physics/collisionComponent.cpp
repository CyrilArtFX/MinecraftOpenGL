#include "collisionComponent.h"
#include "physics.h"
#include <Rendering/material.h>
#include <Utils/color.h>

CollisionComponent::~CollisionComponent()
{
	if (registered)
	{
		Physics::RemoveCollision(this);
	}

	onCollisionDelete.broadcast();
}

void CollisionComponent::setAssociatedTransform(const Transform* newTransform)
{
	associatedTransform = newTransform;
}

bool CollisionComponent::resolvePoint(const Vector3& point) const
{
	bool intersect = resolvePointIntersection(point);
	//intersectedLastFrame = (intersectedLastFrame || intersect);
	return intersect;
}

bool CollisionComponent::resolveRaycast(const Ray& raycast, RaycastHitInfos& outHitInfos) const
{
	bool intersect = resolveRaycastIntersection(raycast, outHitInfos);
	//  it's up to the physics manager to set the intersected last frame
	//  the physics manager also broadcast the onRaycastIntersect event
	return intersect;
}

bool CollisionComponent::resolveCollision(const CollisionComponent& otherCol) const
{
	bool intersect = resolveCollisionIntersection(otherCol);
	//  it's up to the physics manager to set the intersected last frame (for both collisions)
	//  the physics manager also broadcast the onCollisionIntersect event (for both collisions)
	return intersect;
}

void CollisionComponent::drawDebug(Material& debugMaterial) const
{
	debugMaterial.getShader().setVec3("color", intersectedLastFrame ? Color::red : Color::green);
	drawDebugMesh(debugMaterial);
}

const Matrix4 CollisionComponent::getModelMatrix() const
{
	if (!associatedTransform) return Matrix4::identity;

	return associatedTransform->getModelMatrixConst();
}

void CollisionComponent::updateCollisionBeforeTests()
{
	intersectedLastFrame = false;
}

void CollisionComponent::updateCollisionAfterTests()
{
	if (useCCD) posLastFrame = associatedTransform->getPosition();
}

void CollisionComponent::forceIntersected() const
{
	intersectedLastFrame = true;
}

void CollisionComponent::setCCD(bool cdd)
{
	useCCD = cdd;
	if (useCCD) posLastFrame = associatedTransform->getPosition();
}

CollisionComponent::CollisionComponent(CollisionType collisionType_, const Transform* associatedTransform_, Mesh* debugMesh_) :
	collisionType(collisionType_), associatedTransform(associatedTransform_), debugMesh(debugMesh_)
{
}
