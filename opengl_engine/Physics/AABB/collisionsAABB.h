#pragma once
#include "boxAABBColComp.h"

/** Collisions AABB
* Static functions that will check collisions that involves AABB components.
*/
class CollisionsAABB
{
public:
	static bool IntersectPoint(const BoxAABBColComp& boxAABB, const Vector3& point);

	static bool IntersectRaycast(const BoxAABBColComp& boxAABB, const Ray& raycast, RaycastHitInfos& outHitInfos);

	static bool IntersectBoxAABB(const BoxAABBColComp& boxAABB, const BoxAABBColComp& otherBoxAABB);
};

