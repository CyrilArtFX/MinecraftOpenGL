#pragma once
#include <Objects/transform.h>
#include "raycast.h"

#include <Events/ZeroParam/event_zeroParam.h>

#include <Rendering/Model/mesh.h>

class Material;


enum class CollisionType : uint8_t
{
	Null = 0,
	BoxAABB = 1
};


/** Collision Component
* Base class for every collision component
*/
class CollisionComponent
{
public:
	virtual ~CollisionComponent();

	inline CollisionType getCollisionType() const { return collisionType; }

	void setAssociatedTransform(const Transform* newTransform);

	bool resolvePoint(const Vector3& point) const;
	bool resolveRaycast(const Ray& raycast, RaycastHitInfos& outHitInfos) const;
	bool resolveCollision(const CollisionComponent& otherCol) const;

	void drawDebug(Material& debugMaterial) const;

	virtual const Matrix4 getModelMatrix() const;

	void resetIntersected();
	//  for physics manager
	void forceIntersected() const;
	inline bool getIntersected() const { return intersectedLastFrame; }


	//  for physics manager
	bool registered{ false };



	Event_ZeroParam onCollisionDelete;
	Event_ZeroParam onRaycastIntersect;


protected:
	CollisionComponent(CollisionType collisionType_, const Transform* associatedTransform_, Mesh* debugMesh_);

	virtual bool resolvePointIntersection(const Vector3& point) const = 0;
	virtual bool resolveRaycastIntersection(const Ray& raycast, RaycastHitInfos& outHitInfos) const = 0;
	virtual bool resolveCollisionIntersection(const CollisionComponent& otherCol) const = 0;

	virtual void drawDebugMesh(Material& debugMaterial) const = 0;


	CollisionType collisionType{ CollisionType::Null };
	const Transform* associatedTransform{ nullptr };


	//  for debug drawing
	Mesh* debugMesh{ nullptr };

private:
	mutable bool intersectedLastFrame{ false };
};