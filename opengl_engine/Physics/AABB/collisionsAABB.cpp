#include "collisionsAABB.h"
#include <Maths/maths.h>
#include <Physics/physics.h>

using Maths::max;
using Maths::min;

bool CollisionsAABB::IntersectPoint(const BoxAABBColComp& boxAABB, const Vector3& point)
{
	Box box = boxAABB.getTransformedBox();
	return BoxPointIntersection(box, point);
}

bool CollisionsAABB::IntersectLineRaycast(const BoxAABBColComp& boxAABB, const Ray& raycast, RaycastHitInfos& outHitInfos)
{
	Box box = boxAABB.getTransformedBox();

	float hit_distance = 0.0f;
	Vector3 hit_location = Vector3::zero;

	bool intersect = BoxRayIntersection(box, raycast, hit_distance, hit_location);

	//  check if it is the closest collision found
	if (hit_distance < outHitInfos.hitDistance)
	{
		outHitInfos.hitDistance = hit_distance;
		outHitInfos.hitLocation = hit_location;
		outHitInfos.hitNormal = boxAABB.getNormal(hit_location);
		outHitInfos.hitCollision = &boxAABB;
	}

	return intersect;
}

bool CollisionsAABB::IntersectAABBRaycast(const BoxAABBColComp& boxAABB, const Box& raycast)
{
	Box box = boxAABB.getTransformedBox();

	bool intersect = BoxesIntersection(box, raycast);

	return intersect;
}

bool CollisionsAABB::IntersectBoxAABB(const BoxAABBColComp& boxAABB, const BoxAABBColComp& otherBoxAABB)
{
	Box box_a = boxAABB.getTransformedBox();
	Box box_b = otherBoxAABB.getTransformedBox();

	return BoxesIntersection(box_a, box_b);
}

bool CollisionsAABB::CollideBodyBox(const RigidbodyComponent& bodyAABB, CollisionResponse& outBodyResponse, const BoxAABBColComp& boxAABB)
{
	//  if a rigidbody is tested here, it means that it has physics activated

	bool interpolate = false;
	const BoxAABBColComp& body_box_aabb = static_cast<const BoxAABBColComp&>(bodyAABB.getAssociatedCollision());
	Vector3 collision_normal = Vector3::zero;
	Box body_box = body_box_aabb.getTransformedBox();
	const Box static_box = boxAABB.getTransformedBox();

	if (!bodyAABB.getUseCCD())
	{
		body_box.setCenterPoint(body_box.getCenterPoint() + bodyAABB.getAnticipatedMovement());
		interpolate = BoxesIntersection(body_box, static_box);

		if (interpolate)
		{
			//  compute repulsion
			Box minkowski_diff = Box::MinkowskiDifference(body_box, static_box);
			Vector3 repulsion = minkowski_diff.getPointOnPerimeter(Vector3::zero);
			float rep_length = repulsion.length();
			Vector3 body_vel = bodyAABB.getAnticipatedMovement();
			body_vel.normalize();
			repulsion = -body_vel * rep_length;

			//  compute hit location
			body_box.setCenterPoint(body_box.getCenterPoint() + repulsion);
			Vector3 hit_location = body_box.getCenterPoint() + body_vel * body_box.getHalfExtents();

			//  compute collision normal
			collision_normal = boxAABB.getNormal(static_box.getPointOnPerimeter(hit_location));

			//  clamp repulsion to collision normal
			if (!Maths::samesign(collision_normal.x, repulsion.x)) repulsion.x = 0.0f;
			if (!Maths::samesign(collision_normal.y, repulsion.y)) repulsion.y = 0.0f;
			if (!Maths::samesign(collision_normal.z, repulsion.z)) repulsion.z = 0.0f;

			//  set out body response;
			outBodyResponse.repulsion = repulsion;
			outBodyResponse.impactPoint = body_box.getPointOnPerimeter(hit_location);
			outBodyResponse.impactNormal = body_box_aabb.getNormal(outBodyResponse.impactPoint);
		}
	}

	if (bodyAABB.getUseCCD())
	{
		const Vector3 ccd_pos_next_frame = body_box.getCenterPoint() + bodyAABB.getAnticipatedMovement();
		float hit_distance = 0.0f;
		Vector3 hit_location = Vector3::zero;

		interpolate = CCDBoxIntersection(body_box, ccd_pos_next_frame, static_box, hit_distance, hit_location);

		if (interpolate)
		{
			//  compute collision normal
			collision_normal = boxAABB.getNormal(hit_location);

			//  compute replusion
			Vector3 body_vel = bodyAABB.getAnticipatedMovement();
			float body_vel_length = body_vel.length();
			body_vel.normalize();
			Vector3 repulsion = -body_vel * (body_vel_length - hit_distance);

			//  clamp repulsion to collision normal
			if (!Maths::samesign(collision_normal.x, repulsion.x)) repulsion.x = 0.0f;
			if (!Maths::samesign(collision_normal.y, repulsion.y)) repulsion.y = 0.0f;
			if (!Maths::samesign(collision_normal.z, repulsion.z)) repulsion.z = 0.0f;

			//  set out body response;
			outBodyResponse.repulsion = repulsion;
			outBodyResponse.impactPoint = body_box.getPointOnPerimeter(hit_location);
			outBodyResponse.impactNormal = body_box_aabb.getNormal(outBodyResponse.impactPoint);
		}
	}

	//  step (make a moving rigidbody able to step on a collision if it is low enough)
	//  it is identical for ccd and non ccd since step doesn't use ccd
	if (interpolate && !(collision_normal == Vector3::unitY) && body_box.getMinPoint().y + bodyAABB.getStepHeight() > static_box.getMaxPoint().y)
	{
		float y_difference = static_box.getMaxPoint().y - body_box.getMinPoint().y;
		if (y_difference >= 0.0f)
		{
			//  the colliding object could be step up on

			float anticipated_movement_y = bodyAABB.getAnticipatedMovement().y;
			if (anticipated_movement_y < 0.0f) y_difference += -anticipated_movement_y; //  cancel gravity

			//  test if the destination point is collision free
			if (Physics::AABBRaycast(bodyAABB.getAnticipatedMovement() + Vector3{ 0.0f, y_difference + 0.001f, 0.0f }, body_box, 0.0f)) return interpolate;

			const Vector3 body_pos_next_frame = body_box.getCenterPoint() + bodyAABB.getAnticipatedMovement();

			outBodyResponse.repulsion = Vector3{ 0.0f, y_difference, 0.0f };
			outBodyResponse.impactPoint = body_pos_next_frame + Vector3{ 0.0f, y_difference - body_box.getHalfExtents().y, 0.0f };
			outBodyResponse.impactNormal = body_box_aabb.getNormal(outBodyResponse.impactPoint);
		}
	}

	return interpolate;
}

bool CollisionsAABB::CollideBodies(const RigidbodyComponent& bodyAABBa, CollisionResponse& outBodyAResponse, const RigidbodyComponent& bodyAABBb, CollisionResponse& outBodyBResponse)
{
	//  if rigidbodies are tested here, that means that they both have physics activated

	bool interpolate = false;
	const BoxAABBColComp& body_a_box_aabb = static_cast<const BoxAABBColComp&>(bodyAABBa.getAssociatedCollision());
	const BoxAABBColComp& body_b_box_aabb = static_cast<const BoxAABBColComp&>(bodyAABBb.getAssociatedCollision());
	Box body_a_box = body_a_box_aabb.getTransformedBox();
	Box body_b_box = body_b_box_aabb.getTransformedBox();

	if (!bodyAABBa.getUseCCD() && !bodyAABBb.getUseCCD()) //  no ccd
	{
		body_a_box.setCenterPoint(body_a_box.getCenterPoint() + bodyAABBa.getAnticipatedMovement());
		body_b_box.setCenterPoint(body_b_box.getCenterPoint() + bodyAABBb.getAnticipatedMovement());
		interpolate = BoxesIntersection(body_a_box, body_b_box);

		if (interpolate)
		{
			//  compute repulsion
			Box minkowski_diff = Box::MinkowskiDifference(body_a_box, body_b_box);
			Vector3 repulsion_minkowski = minkowski_diff.getPointOnPerimeter(Vector3::zero);
			float rep_length = repulsion_minkowski.length();

			Vector3 body_vel_a = bodyAABBa.getAnticipatedMovement();
			body_vel_a.normalize();
			Vector3 body_vel_b = bodyAABBb.getAnticipatedMovement();
			body_vel_b.normalize();

			float total_weights = bodyAABBa.getWeight() + bodyAABBb.getWeight();

			Vector3 repulsion_a = -body_vel_a * rep_length * (bodyAABBb.getWeight() / total_weights);
			Vector3 repulsion_b = -body_vel_b * rep_length * (bodyAABBa.getWeight() / total_weights);

			//  compute hit location
			body_a_box.setCenterPoint(body_a_box.getCenterPoint() + repulsion_a);
			Vector3 hit_location_a = body_a_box.getCenterPoint() + body_vel_a * body_a_box.getHalfExtents();
			body_b_box.setCenterPoint(body_b_box.getCenterPoint() + repulsion_b);
			Vector3 hit_location_b = body_b_box.getCenterPoint() + body_vel_b * body_b_box.getHalfExtents();

			//  compute collision normal
			Vector3 collision_normal_a = body_b_box_aabb.getNormal(body_b_box.getPointOnPerimeter(hit_location_b));
			Vector3 collision_normal_b = body_a_box_aabb.getNormal(body_a_box.getPointOnPerimeter(hit_location_a));

			//  clamp repulsion to collision normal
			if (!Maths::samesign(collision_normal_a.x, repulsion_a.x)) repulsion_a.x = 0.0f;
			if (!Maths::samesign(collision_normal_a.y, repulsion_a.y)) repulsion_a.y = 0.0f;
			if (!Maths::samesign(collision_normal_a.z, repulsion_a.z)) repulsion_a.z = 0.0f;
			if (!Maths::samesign(collision_normal_b.x, repulsion_b.x)) repulsion_b.x = 0.0f;
			if (!Maths::samesign(collision_normal_b.y, repulsion_b.y)) repulsion_b.y = 0.0f;
			if (!Maths::samesign(collision_normal_b.z, repulsion_b.z)) repulsion_b.z = 0.0f;

			//  set out body response;
			outBodyAResponse.repulsion = repulsion_a;
			outBodyAResponse.impactPoint = body_a_box.getPointOnPerimeter(hit_location_a);
			outBodyAResponse.impactNormal = body_a_box_aabb.getNormal(outBodyAResponse.impactPoint);
			outBodyBResponse.repulsion = repulsion_b;
			outBodyBResponse.impactPoint = body_b_box.getPointOnPerimeter(hit_location_b);
			outBodyBResponse.impactNormal = body_b_box_aabb.getNormal(outBodyBResponse.impactPoint);
		}
	}
	else //  ccd
	{

	}

	return interpolate;
}


bool CollisionsAABB::BoxesIntersection(const Box& boxA, const Box& boxB)
{
	Box minkowski = Box::MinkowskiDifference(boxA, boxB);

	return BoxPointIntersection(minkowski, Vector3::zero);
}

bool CollisionsAABB::BoxPointIntersection(const Box& box, const Vector3& point)
{
	Vector3 box_min = box.getMinPoint();
	Vector3 box_max = box.getMaxPoint();

	return
		point.x >= box_min.x && point.x <= box_max.x &&
		point.y >= box_min.y && point.y <= box_max.y &&
		point.z >= box_min.z && point.z <= box_max.z;
}

bool CollisionsAABB::BoxRayIntersection(const Box& box, const Ray& ray, float& distance, Vector3& location, bool computeCollision)
{
	Vector3 box_min = box.getMinPoint();
	Vector3 box_max = box.getMaxPoint();

	Vector3 ray_origin = ray.getOrigin();
	Vector3 ray_direction = ray.getDirection();
	float ray_length = ray.getLength();

	Vector3 dirfrac;
	dirfrac.x = 1.0f / ray_direction.x;
	dirfrac.y = 1.0f / ray_direction.y;
	dirfrac.z = 1.0f / ray_direction.z;

	float t1 = (box_min.x - ray_origin.x) * dirfrac.x;
	float t2 = (box_max.x - ray_origin.x) * dirfrac.x;
	float t3 = (box_min.y - ray_origin.y) * dirfrac.y;
	float t4 = (box_max.y - ray_origin.y) * dirfrac.y;
	float t5 = (box_min.z - ray_origin.z) * dirfrac.z;
	float t6 = (box_max.z - ray_origin.z) * dirfrac.z;

	float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
	float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

	distance = std::numeric_limits<float>::max();

	//  if tmax == 0, origin of the ray is on the edge of AABB
	//  it is fine that it return true for most cases, but when using raycasts for computing collisions, it should return false
	if (tmax == 0.0f && computeCollision)
	{
		return false;
	}

	//  if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
	if (tmax < 0.0f)
	{
		return false;
	}

	//  if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax)
	{
		return false;
	}

	//  if tmin > length, ray (line) is intersecting AABB, but the whole AABB is after the end of the raycast
	if (tmin > ray_length)
	{
		return false;
	}

	distance = tmin;
	location = ray_origin + ray_direction * distance;

	return true;
}

bool CollisionsAABB::CCDBoxIntersection(const Box& boxCCD, const Vector3& ccdNextFramePos, const Box& box, float& distance, Vector3& location)
{
	Vector3 box_ccd_pos = boxCCD.getCenterPoint();

	if (box_ccd_pos == ccdNextFramePos)
	{
		//  if the ccd_box is not moving, that means it's movement has already been completely reversed by another collision this frame,
		//  so it will be at the exact same place that previous frame, where it wasn't colliding with any static collision so it can't collide with this one
		return false;
	}

	Box box_static = box;
	box_static.addHalfExtents(boxCCD);

	Ray ray;
	ray.setupWithStartEnd(box_ccd_pos, ccdNextFramePos);

	bool intersect = BoxRayIntersection(box_static, ray, distance, location, true);

	//  with this way of compute ccd / static box intersection, the location returned by the raycast is equivalent to the center of the ccd box
	//  so compute the location again so that it at the real intersection point between the two boxes
	Vector3 direction = ray.getDirection();
	direction.normalize();
	location += direction * boxCCD.getHalfExtents();
	location = box.getPointOnPerimeter(location);

	return intersect;
}

bool CollisionsAABB::CCDsIntersection(const Box& boxACCD, const Vector3& ccdANextFramePos, const Box& boxBCCD, const Vector3& ccdBNextFramePos, float& distanceA, float& distanceB, Vector3& location)
{
	return false;
}
