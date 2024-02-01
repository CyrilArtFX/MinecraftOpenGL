#include "physics.h"

#include <iostream>
#include <algorithm>


std::vector<CollisionComponent*> Physics::collisionsComponents;
std::vector<RigidbodyComponent*> Physics::rigidbodiesComponents;
std::vector<Raycast*> Physics::raycasts;


CollisionComponent& Physics::CreateCollisionComponent(CollisionComponent* colComp)
{
	std::cout << "PHYSICS_INFO: Create a collision.\n";
	collisionsComponents.push_back(colComp);

	CollisionComponent& col = *(collisionsComponents.back());
	col.registered = true;
	return col;
}

void Physics::RemoveCollision(CollisionComponent* colComp)
{
	auto iter = std::find(collisionsComponents.begin(), collisionsComponents.end(), colComp);
	if (iter == collisionsComponents.end())
	{
		std::cout << "PHYSICS_WARNING: Couldn't find a collision to remove.\n";
		return;
	}

	std::iter_swap(iter, collisionsComponents.end() - 1);
	CollisionComponent& col = *(collisionsComponents.back());
	col.registered = false;
	collisionsComponents.pop_back();

	std::cout << "PHYSICS_INFO: Successfully removed a collision.\n";
}

RigidbodyComponent& Physics::CreateRigidbodyComponent(RigidbodyComponent* rigidbodyComp)
{
	std::cout << "PHYSICS_INFO: Create a rigidbody.\n";
	rigidbodiesComponents.push_back(rigidbodyComp);

	RigidbodyComponent& rigidbody = *(rigidbodiesComponents.back());
	rigidbody.registered = true;
	return rigidbody;
}

void Physics::RemoveRigidbody(RigidbodyComponent* rigidbodyComp)
{
	auto iter = std::find(rigidbodiesComponents.begin(), rigidbodiesComponents.end(), rigidbodyComp);
	if (iter == rigidbodiesComponents.end())
	{
		std::cout << "PHYSICS_WARNING: Couldn't find a rigidbody to remove.\n";
		return;
	}

	std::iter_swap(iter, rigidbodiesComponents.end() - 1);
	RigidbodyComponent& rigidbody = *(rigidbodiesComponents.back());
	rigidbody.registered = false;
	rigidbodiesComponents.pop_back();

	std::cout << "PHYSICS_INFO: Successfully removed a rigidbody.\n";
}

bool Physics::RaycastLine(const Vector3& start, const Vector3& end, RaycastHitInfos& outHitInfos, float drawDebugTime)
{
	std::cout << "PHYSICS_INFO: Create a raycast line.\n";

	outHitInfos = RaycastHitInfos();

	bool hit = false;

	if (drawDebugTime != 0.0f)
	{
		raycasts.emplace_back(new Raycast(start, end, drawDebugTime));

		const Ray& ray = raycasts.back()->getRay();

		for (auto& col : collisionsComponents)
		{
			bool col_hit = col->resolveRaycast(ray, outHitInfos);
			hit = hit || col_hit;
		}

		if (outHitInfos.hitCollision)
		{
			outHitInfos.hitCollision->forceIntersected();
			outHitInfos.hitCollision->onRaycastIntersect.broadcast(outHitInfos.hitLocation);
		}


		if (hit) raycasts.back()->setHitPos(outHitInfos.hitLocation);

		return hit;
	}
	else //  do not register the raycast in the list if it will not draw debug
	{
		Raycast* raycast = new Raycast(start, end, drawDebugTime);

		const Ray& ray = raycast->getRay();

		for (auto& col : collisionsComponents)
		{
			bool col_hit = col->resolveRaycast(ray, outHitInfos);
			hit = hit || col_hit;
		}

		if (outHitInfos.hitCollision)
		{
			//  also do not set the collision intersected if it will not draw debug
			outHitInfos.hitCollision->onRaycastIntersect.broadcast(outHitInfos.hitLocation);
		}

		delete raycast;

		//  finally no need to set the hit pos on the raycast if it will not draw debug

		return hit;
	}
}

void Physics::UpdatePhysics(float dt)
{
	//  reset the 'intersected last frame' parameter
	for (auto& col : collisionsComponents)
	{
		col->resetIntersected();
	}

	//  delete raycasts that have run out of time
	for (int i = 0; i < raycasts.size(); i++)
	{
		raycasts[i]->updateDrawDebugTimer(dt);

		if (raycasts[i]->drawDebugTimerFinished())
		{
			delete raycasts[i];

			std::iter_swap(raycasts.begin() + i, raycasts.end() - 1);
			raycasts.pop_back();

			i--;
		}
	}

	//  test the currently existing raycasts for debug drawing
	for (auto& raycast : raycasts)
	{
		RaycastHitInfos out = RaycastHitInfos();
		for (auto& col : collisionsComponents)
		{
			col->resolveRaycast(raycast->getRay(), out);
		}
		if (out.hitCollision) out.hitCollision->forceIntersected();
	}

	//  test all the rigidbodies
	for (int i = 0; i < rigidbodiesComponents.size(); i++)
	{
		RigidbodyComponent& rigidbody = *rigidbodiesComponents[i];
		if (!rigidbody.isAssociatedCollisionValid()) continue;

		//  test rigidbody / collisions
		for (int j = 0; j < collisionsComponents.size(); j++)
		{
			CollisionComponent& col = *collisionsComponents[j];
			bool hit = false;
			if (rigidbody.getUseCCD())
			{
				hit = col.resolveCollisionCCD(rigidbody.getAssociatedCollision(), false);
			}
			else
			{
				hit = col.resolveCollision(rigidbody.getAssociatedCollision());
			}

			if (hit)
			{
				rigidbody.getAssociatedCollision().forceIntersected();
				col.forceIntersected();
				rigidbody.onCollisionIntersect.broadcast();
			}
		}

		//  test rigidbody / other rigidbodies
		if (i = rigidbodiesComponents.size() - 1) return;
		for (int k = i + 1; k < rigidbodiesComponents.size(); k++)
		{
			RigidbodyComponent& other_rigidbody = *rigidbodiesComponents[k];
			if (!other_rigidbody.isAssociatedCollisionValid()) continue;
			bool hit = false;
			if (rigidbody.getUseCCD())
			{
				if (other_rigidbody.getUseCCD())
				{
					hit = other_rigidbody.getAssociatedCollision().resolveCollisionCCD(rigidbody.getAssociatedCollision(), true);
				}
				else
				{
					hit = other_rigidbody.getAssociatedCollision().resolveCollisionCCD(rigidbody.getAssociatedCollision(), false);
				}
			}
			else
			{
				if (other_rigidbody.getUseCCD())
				{
					hit = rigidbody.getAssociatedCollision().resolveCollisionCCD(other_rigidbody.getAssociatedCollision(), false);
				}
				else
				{
					hit = rigidbody.getAssociatedCollision().resolveCollision(other_rigidbody.getAssociatedCollision());
				}
			} //  not beautiful but it works

			if (hit)
			{
				rigidbody.getAssociatedCollision().forceIntersected();
				other_rigidbody.getAssociatedCollision().forceIntersected();
				rigidbody.onCollisionIntersect.broadcast();
				other_rigidbody.onCollisionIntersect.broadcast();
			}
		}
		
	}

	//  update the 'pos last frame'
	for (auto& rigidbody : rigidbodiesComponents)
	{
		rigidbody->updatePosLastFrame();
	}
}

void Physics::ClearAllCollisions()
{
	std::cout << "PHYSICS_INFO: Clearing all collisions, rigidbodies and raycasts.\n";

	for (auto col : collisionsComponents)
	{
		col->registered = false;
		delete col;
	}
	collisionsComponents.clear();

	for (auto rigidbody : rigidbodiesComponents)
	{
		rigidbody->registered = false;
		delete rigidbody;
	}
	rigidbodiesComponents.clear();

	for (auto raycast : raycasts)
	{
		delete raycast;
	}
	raycasts.clear();
}

void Physics::DrawCollisionsDebug(Material& debugMaterial)
{
	for (auto& col : collisionsComponents)
	{
		col->drawDebug(debugMaterial);
	}

	for (auto& rigidbody : rigidbodiesComponents)
	{
		rigidbody->getAssociatedCollision().drawDebug(debugMaterial);
	}

	for (auto& raycast : raycasts)
	{
		raycast->drawDebugRaycast(debugMaterial);
	}
}
