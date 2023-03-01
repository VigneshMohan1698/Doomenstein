#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Game/ActorUID.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Renderer/SpriteAnimationGroupDefinition.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Core/Clock.hpp"

class ActorDefinition;
class SpawnInfo;
class Weapon;
class Controller;
class Game;
class Map;
class AI;


class Actor
{
public:
	Actor(Map* map, const SpawnInfo& spawnInfo);
	Actor(Game* game, Vec3 position, EulerAngles orientationDegrees, EulerAngles angularVelocity, float height, float radius);
	~Actor();

	void Update(float deltaSeconds);
	void UpdatePhysics(float deltaSeconds);
	void UpdateAnimations(float deltaSeconds);
	void CreateWeaponsFromInventory();
	void Render() const;
	void AddVertsForActor();
	void PlayAnimationGroupByName(std::string name);
	Mat44 GetModalMatrix() const;
	void CheckForActorCollisionsAgainstWall();
	void CheckForActorCollisionsAgainstOtherActors();
	Vec3 GetForward() const;
	Vec3 GetJForward() const;
	void ClampActorRotation();
	void Damage(float damage);
	void Die();

	void AddForce(const Vec3& force);
	void AddImpulse(const Vec3& impulse);
	void OnCollide(Actor* other);

	void OnPossessed(Controller* controller);
	void OnUnpossessed(Controller* controller);
	void MoveInDirection(Vec3 direction, float speed);
	void TurnInDirection(EulerAngles direction, float deltaSeconds);

	Weapon* GetEquippedWeapon();

	void EquipWeapon(int weaponIndex);
	void EquipNextWeapon();
	Weapon* GetWeapon(int weaponIndex);
	void EquipPreviousWeapon();
	void Attack();
public:
	Game* m_game;
	ActorUID m_uid = ActorUID::INVALID;
	const ActorDefinition* m_definition = nullptr;
	Map* m_map = nullptr;
	AABB2 actorQuad;

	std::vector<Vertex_PCU> m_vertices;
	std::vector<Vertex_PCU> m_wireFrameVertices;

	Rgba8 m_color = Rgba8::WHITE;
	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientationDegrees = EulerAngles::ZERO;
	Vec3 m_velocity = Vec3::ZERO;
	EulerAngles m_angularVelocity = EulerAngles::ZERO;
	Vec3 m_acceleration = Vec3::ZERO;
	float m_physicsRadius = 1.0f;
	float m_physicsHeight = 1.0f;

	std::vector<Weapon*> m_weapons;
	int m_equippedWeaponIndex = 0;

	Actor* m_owner = nullptr;
	bool m_isDestroyed = false;
	bool m_isDead = false;
	bool m_isHurt = false;
	bool m_isWalking = false;
	bool m_isAttacking = false;

	float m_health = 200.0f;
	Stopwatch m_lifetimeStopwatch;

	Controller* m_controller = nullptr;
	AI* m_aiController = nullptr;

	bool m_renderForward = false;
	Rgba8 m_solidColor = Rgba8(192, 0, 0,255);
	Rgba8 m_wireframeColor = Rgba8(255, 192, 192, 255);
	float m_currentAnimationSeconds = 1.0f;

	Stopwatch m_animationStopwatch;
	Clock     m_animationClock;
	const SpriteAnimationGroupDefinition* m_currentAnimation = nullptr;
};