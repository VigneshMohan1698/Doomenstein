#include "Game/AI.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include "Game/Actor.hpp"
#include <Engine/Math/RandomNumberGenerator.hpp>
#include "Game/Weapon.hpp"
AI::AI()
{
}

AI::~AI()
{
}

void AI::Update(float deltaSeconds)
{
	RandomNumberGenerator rng;
	Actor* possessedActor = GetActor();
	if (possessedActor != nullptr)
	{
		float goalOrientationYaw = 0.0f;
		Player* closestPlayer = GetMap()->GetClosestVisiblePlayer(possessedActor);
		if (closestPlayer == nullptr || closestPlayer->GetActor() == nullptr)
		{
			return;
		}
		Vec3 playerPosition = closestPlayer->GetActor()->m_position;
		Vec3 direction = playerPosition - possessedActor->m_position;

		Vec2 impactAnglePosition = Vec2(closestPlayer->GetActor()->m_position) - Vec2(possessedActor->m_position);
		goalOrientationYaw = impactAnglePosition.GetOrientationDegrees();

		EulerAngles newOrientation = EulerAngles(goalOrientationYaw, possessedActor->m_orientationDegrees.m_pitchDegrees, possessedActor->m_orientationDegrees.m_rollDegrees);
		possessedActor->TurnInDirection(newOrientation, deltaSeconds);
		float runSpeed = possessedActor->m_definition->m_runSpeed;
		float walkSpeed = possessedActor->m_definition->m_walkSpeed;
		float meleeRange = possessedActor->m_definition->m_meleeRange;
		float distance = GetDistance3D(possessedActor->m_position, closestPlayer->GetActor()->m_position);
		if (distance >= (meleeRange + possessedActor->m_physicsRadius))
		{
			if (distance > 5.0f)
			{
				possessedActor->MoveInDirection(direction.GetNormalized(), runSpeed);
			}
			else
			{
				possessedActor->MoveInDirection(direction.GetNormalized(), walkSpeed);
			}
			if (m_blueFireStopwatch.HasDurationElapsed() && possessedActor->m_definition->m_name == "FlyingDemon")
			{
				Vec3 positionStart = possessedActor->m_position;
				positionStart.z = 0.5f;
				m_blueFireStopwatch.Start(possessedActor->m_definition->m_meleeDelay);
				possessedActor->m_weapons[2]->Fire(positionStart, positionStart, possessedActor->GetForward(), possessedActor);
			}
			
		}
		else if(m_meleeStopwatch.HasDurationElapsed())
		{
			m_meleeStopwatch.Start(possessedActor->m_definition->m_meleeDelay);
			possessedActor->Attack();
			float damage = rng.GetRandomFloatInRange(possessedActor->m_definition->m_meleeDamage.m_min, possessedActor->m_definition->m_meleeDamage.m_max);
			closestPlayer->GetActor()->Damage(damage);
		}
	}
}
