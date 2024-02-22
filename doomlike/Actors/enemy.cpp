#include "enemy.h"
#include <Physics/physics.h>
#include <Assets/assetManager.h>

#include "Player.h"
#include <doomlikeGame.h>
#include <GameplayStatics/gameplayStatics.h>

#include <iostream>


void Enemy::load()
{
	addModel(&AssetManager::GetModel("enemy"));

	rigidbody = &Physics::CreateRigidbodyComponent(new RigidbodyComponent(new BoxAABBColComp(Box{Vector3::zero, Vector3{0.7f, 0.7f, 0.7f}}, this, false, "enemy"), false, true));
	rigidbody->getAssociatedCollisionNonConst().onCollisionIntersect.registerObserver(this, Bind_1(&Enemy::onBodyIntersect));

	rigidbody->setUseGravity(false);

	setScale(0.7f);

	playerRef = static_cast<DoomlikeGame*>(GameplayStatics::GetGame())->getPlayer();
}

void Enemy::updateObject(float dt)
{
	if (dead)
	{
		delete rigidbody;
		GameplayStatics::GetActiveScene()->unregisterObject(this);
		return;
	}

	if (!playerRef) return;

	RaycastHitInfos out;
	bool test_player = Physics::LineRaycast(getPosition(), playerRef->getEyePosition(), { "solid", "player" }, out, 0.0f);
	if (!test_player) return;

	if (out.hitDistance > range) return;

	if (out.hitCollision->getCollisionChannel() != "player") return;

	rotateTowards(playerRef->getPosition());
}

void Enemy::onBodyIntersect(RigidbodyComponent& other)
{
	if (other.getAssociatedCollision().getCollisionChannel() == "bullet")
	{
		std::cout << "Enemy Die\n";
		dead = true;
		//  not safe to delete rigidbody here cause it's during the update physics step
		//  might need a way to automate this with a pending system
	}
}