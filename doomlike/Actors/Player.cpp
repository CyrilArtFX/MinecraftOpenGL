#include "Player.h"

#include <Inputs/input.h>
#include <Maths/maths.h>
#include <Physics/physics.h>
#include <Physics/ObjectChannels/collisionChannels.h>
#include <Assets/assetManager.h>
#include <Rendering/renderer.h>

#include <GameplayStatics/gameplayStatics.h>
#include <doomlikeGame.h>

#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>


Player::Player() :
	rigidbody(&Physics::CreateRigidbodyComponent(new RigidbodyComponent(new BoxAABBColComp(Box::one, this, true, "player", false), true, false)))
{
	rigidbody->onRigidbodyDelete.registerObserver(this, Bind_0(&Player::onRigidbodyDeleted));
	rigidbody->onCollisionRepulsed.registerObserver(this, Bind_1(&Player::onCollision));
}

void Player::setup(float height, float speed, float jump, float stepHeight, Renderer* renderer)
{
	camHeight = height;
	moveSpeed = speed;
	jumpForce = jump;
	rendererRef = renderer;

	rigidbody->setStepHeight(stepHeight);

	gunObject.addModel(&AssetManager::GetModel("gun"));
	rendererRef->addObject(&gunObject);
	gunObject.setScale(0.1f);

	setPosition(0.0f, 0.0f, 0.0f);
	camPos = Vector3{ 0.0f, camHeight, 0.0f };
	camera.setPosition(camPos);
	camera.setSensitivity(0.08f);

	BoxAABBColComp& collision = static_cast<BoxAABBColComp&>(rigidbody->getAssociatedCollisionNonConst());
	collision.changeBox(Box{ Vector3{0.0f, (camHeight / 2.0f) + 0.1f, 0.0f}, Vector3{0.3f, (camHeight / 2.0f) + 0.1f, 0.3f} });
	rigidbody->setPhysicsActivated(true);
	rigidbody->setUseGravity(true);

	rigidbody->setTestChannels(CollisionChannels::GetRegisteredTestChannel("PlayerEntity"));
}


void Player::update(float dt)
{
	//  move player
	Vector3 velocity_xz = Vector3::zero;

	if (Input::IsKeyDown(GLFW_KEY_W))
		velocity_xz += camera.getFlatFront() * moveSpeed;

	if (Input::IsKeyDown(GLFW_KEY_S))
		velocity_xz -= camera.getFlatFront() * moveSpeed;

	if (Input::IsKeyDown(GLFW_KEY_A))
		velocity_xz += camera.getRight() * moveSpeed;

	if (Input::IsKeyDown(GLFW_KEY_D))
		velocity_xz -= camera.getRight() * moveSpeed;

	//  clamp the velocity to max movement speed
	velocity_xz.clampMagnitude(moveSpeed);

	//  apply velocity to rigidbody
	rigidbody->setVelocity(velocity_xz);


	//  jump
	if (Input::IsKeyPressed(GLFW_KEY_SPACE) && rigidbody->isOnGround())
	{
		rigidbody->addGravityVelocity(Vector3::unitY * jumpForce);
	}


	//  shoot
	if (Input::IsKeyPressed(GLFW_MOUSE_BUTTON_LEFT))
	{
		if (rendererRef)
		{
			RaycastHitInfos out;
			bool ray_hit = Physics::LineRaycast(camera.getPosition(), camera.getPosition() + camera.getForward() * 1000.0f, CollisionChannels::GetRegisteredTestChannel("PlayerEntity"), out, 0.0f);

			Quaternion bullet_rotation;
			Vector3 bullet_direction;
			if (!ray_hit)
			{
				bullet_rotation = Quaternion::concatenate(camera.getRotation(), Quaternion{ camera.getUp(), Maths::toRadians(90.0f) });
				bullet_direction = camera.getForward();
			}
			else
			{
				bullet_rotation = Quaternion::createLookAt(gunObject.getPosition(), out.hitLocation, Vector3::unitY);
				bullet_direction = Vector3::normalize(out.hitLocation - gunObject.getPosition());
			}

			bullets.push_back(std::make_unique<Bullet>(gunObject.getPosition(), bullet_rotation, bullet_direction, shootVelocity, bulletLifeTime, rendererRef));
		}
		else
		{
			std::cout << "Doomlike game error : Player has not his renderer ref setup !\n";
		}
	}


	//  shoot raycast
	if (Input::IsKeyPressed(GLFW_MOUSE_BUTTON_RIGHT))
	{
		Vector3 raycast_start = camera.getPosition();
		Vector3 raycast_end = raycast_start + camera.getForward() * 5.0f;

		//Physics::LineRaycast(raycast_start, raycast_end, CollisionChannels::GetRegisteredTestChannel("PlayerEntity"));
		Physics::AABBSweepRaycast(raycast_start, raycast_end, Box{ Vector3::zero, Vector3{0.1f, 0.1f, 0.1f} }, CollisionChannels::GetRegisteredTestChannel("PlayerEntity"));
	}


	Vector2 mouse_delta = Input::GetMouseDelta(); 
	camera.freecamMouseMovement(mouse_delta.x, mouse_delta.y); 

	//  camera lag
	Vector3 target_cam_pos = getPosition() + Vector3{ 0.0f, camHeight, 0.0f }; //  head of the player (where the cam should be)
	float cam_dist_target = Vector3::Distance(camPos, target_cam_pos); //  distance between the actual cam pos (end of last frame) and target for this frame
	
	if (cam_dist_target > camMaxDist) //  if distance is superior to max distance
	{
		Vector3 target_to_cam_dir = Vector3::normalize(target_cam_pos - camPos); //  direction from target to actual
		camPos = target_cam_pos - target_to_cam_dir * camMaxDist; //  force change actual pos so it works with max distance
		cam_dist_target = camMaxDist; // actualise distance
	}

	float cam_progress = Maths::min(camSpeed * dt / cam_dist_target, 1.0f); //  compute progress with cam speed and distance
	camPos = Vector3::lerp(camPos, target_cam_pos, cam_progress); //  lerp cam pos with progress
	camera.setPosition(camPos);


	//  bullets update
	for (auto& bullet : bullets)
	{
		bullet->update(dt);
	}
	
	for (int i = 0; i < bullets.size(); i++)
	{
		if (bullets[i]->isLTOver())
		{
			bullets[i]->destroy();

			std::iter_swap(bullets.begin() + i, bullets.end() - 1);
			bullets.pop_back();

			break; //  assume that we can't create 2 bullets on the same frame so 2 bullets cannot be destroyed by time out on the same frame
		}
	}



	if (getPosition().y < -50.0f)
	{
		std::cout << "Player die by falling.\n";
		static_cast<DoomlikeGame*>(GameplayStatics::GetGame())->restartLevel();
	}



	//  move gun object to follow player
	gunObject.setPosition(camPos);
	gunObject.addPositionRotated(Vector3{ 0.1f, -0.1f, -0.2f }); //  gun offset of camera
	gunObject.setRotation(camera.getRotation());
	gunObject.incrementRotation(Quaternion{ gunObject.getUp(), Maths::toRadians(180.0f) }); //  gun is rotated backward by default
}

Vector3 Player::getEyePosition() const
{
	return camera.getPosition();
}

void Player::unload()
{
	for (auto& bullet : bullets)
	{
		bullet->destroy();
	}
	bullets.clear();
}

void Player::respawn(PlayerSpawnPoint& spawnPoint)
{
	unload();

	setPosition(spawnPoint.spawnPosition);
	//setRotation(spawnPoint.spawnRotation);

	camPos = spawnPoint.spawnPosition + Vector3{ 0.0f, camHeight, 0.0f };
	camera.setPosition(camPos);
	//camera.setRotByQuaternion(spawnPoint.spawnRotation);
}

void Player::onRigidbodyDeleted()
{
	rigidbody = nullptr;
}

void Player::onCollision(const CollisionResponse& collisionResponse)
{
}
