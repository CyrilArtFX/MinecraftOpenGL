#include "wall.h"
#include <Physics/physics.h>
#include <Physics/AABB/boxAABBColComp.h>


Wall::Wall(Vector3 position, FacingDirection facingDirection, Vector2 scale, bool hasCollision)
{
	setup(position, facingDirection, scale, hasCollision);
}

void Wall::load()
{
	addModel(&AssetManager::GetModel("wall"));
}

void Wall::setup(Vector3 position, FacingDirection facingDirection, Vector2 scale, bool hasCollision)
{
	setPosition(position);
	setScale(scale.x, 1.0f, scale.y);
	scale *= 0.5f;


	switch (facingDirection)
	{
	case FacingDirection::FacingPositiveX:
		setRotation(Quaternion::fromEuler(0.0f, Maths::toRadians(90.0f), Maths::toRadians(-90.0f)));
		if (hasCollision)
			Physics::CreateCollisionComponent(new BoxAABBColComp(Box{ Vector3{-0.01f, 0.0f, 0.0f}, Vector3{0.1f, scale.y, scale.x} }, this, false, "solid", false));
		break;

	case FacingDirection::FacingNegativeX:
		setRotation(Quaternion::fromEuler(0.0f, Maths::toRadians(90.0f), Maths::toRadians(90.0f)));
		if (hasCollision)
			Physics::CreateCollisionComponent(new BoxAABBColComp(Box{ Vector3{0.01f, 0.0f, 0.0f}, Vector3{0.1f, scale.y, scale.x} }, this, false, "solid", false));
		break;

	case FacingDirection::FacingPositiveZ:
		setRotation(Quaternion::fromEuler(Maths::toRadians(90.0f), Maths::toRadians(90.0f), Maths::toRadians(90.0f)));
		if (hasCollision)
			Physics::CreateCollisionComponent(new BoxAABBColComp(Box{ Vector3{0.0f, 0.0f, -0.01f}, Vector3{scale.x, scale.y, 0.1f} }, this, false, "solid", false));
		break;

	case FacingDirection::FacingNegativeZ:
		setRotation(Quaternion::fromEuler(Maths::toRadians(-90.0f), Maths::toRadians(90.0f), Maths::toRadians(90.0f)));
		if (hasCollision)
			Physics::CreateCollisionComponent(new BoxAABBColComp(Box{ Vector3{0.0f, 0.0f, 0.01f}, Vector3{scale.x, scale.y, 0.1f} }, this, false, "solid", false));
		break;
	}
}