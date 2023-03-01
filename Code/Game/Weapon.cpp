#include "Game/Weapon.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"

extern AudioSystem* g_theAudioSystem;

Weapon::Weapon(const WeaponDefinition* definition)
{
    m_definition = definition;
    m_refireStopwatch = Stopwatch(Clock::GetSystemClockPointer(), m_definition->m_refireTime);
}

Weapon::~Weapon()
{
}

void Weapon::Fire(const Vec3& rayPosition, const Vec3& projectileSpawnPosition, const Vec3& forward, Actor* owner)
{
    RandomNumberGenerator rng;
    Vec3 impulse;
    if (!m_refireStopwatch.HasDurationElapsed())
    {
        return;
    }
    Map* map;
    map = owner->m_map;
    if (m_definition->m_name == "Pistol")
    {
        for (int i = 0; i < m_definition->m_numRays; i++)
        {
            float rayDistance = m_definition->m_rayRange;
            RaycastResult resultVsActor;
            RaycastResult resultVsWall;
            RaycastResult result;
            RaycastFilter filter;
            filter.m_ignoreActor = owner;
            filter.m_factionToIgnore = owner->m_definition->m_faction;
            result = map->RaycastAll(rayPosition, forward, rayDistance, filter);
            if (result.m_didImpact && result.m_impactActor != nullptr)
            {
                float damage = rng.GetRandomFloatInRange(m_definition->m_rayDamage.m_min, m_definition->m_rayDamage.m_max);
                result.m_impactActor->Damage(damage);
            }
            if (result.m_didImpact)
            {
                if (result.m_impactActor != nullptr)
                {
                    float randomDamage = rng.GetRandomFloatInRange(m_definition->m_rayDamage.m_min, m_definition->m_rayDamage.m_max);
                    result.m_impactActor->Damage(randomDamage);
                    impulse = (result.m_impactPosition - rayPosition).GetNormalized();
                    impulse.z = 0.0f;
                    result.m_impactActor->AddImpulse(impulse * m_definition->m_rayImpulse);

					SpawnInfo info;
					info.m_position = result.m_impactPosition;
					info.m_definition = ActorDefinition::GetByName("BloodSplatter");
                    owner->Attack();
					map->SpawnActor(info);
                  
                }
                else
				{
					SpawnInfo info;
					info.m_position = result.m_impactPosition;
					info.m_definition = ActorDefinition::GetByName("BulletHit");
                    owner->Attack();
					map->SpawnActor(info);

                }
            }
        }
    }
    else if (m_definition->m_name == "PlasmaRifle")
    {
        for (int i = 0; i < m_definition->m_numProjectiles; i++)
        {
            SpawnInfo info;
            info.m_position = projectileSpawnPosition;
            info.m_definition = m_definition->m_projectileActorDefinition;
            info.m_velocity = GetRandomDirectionInCone(projectileSpawnPosition, forward, m_definition->m_projectileCone) * m_definition->m_projectileSpeed;
            Actor* actor = map->SpawnActor(info);
            actor->m_owner = owner;
        }
    }
	else if (m_definition->m_name == "Firethrower")
	{
		for (int i = 0; i < m_definition->m_numProjectiles; i++)
		{
			SpawnInfo info;
			info.m_position = projectileSpawnPosition;
			info.m_definition = m_definition->m_projectileActorDefinition;
            info.m_velocity = forward * m_definition->m_projectileSpeed;
			Actor* actor = map->SpawnActor(info);
			actor->m_owner = owner;
		}
	}
    g_theAudioSystem->StartSoundAt(m_definition->m_fireSound, owner->m_position);
    m_refireStopwatch.Start(m_definition->m_refireTime);
    return;
}

Vec3 Weapon::GetRandomDirectionInCone(const Vec3& position, const Vec3& forward, float angle) const
{
    UNUSED((void)position);
    Vec3 returnForward;
    RandomNumberGenerator rng;
    float randomAngle = rng.GetRandomFloatInRange(-angle, angle);
    returnForward = forward.GetRotatedAboutZDegrees(randomAngle).GetNormalized();
    return returnForward;
}
