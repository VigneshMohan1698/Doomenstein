#pragma once
#include <vector>
#include <string>
#include "Game/GameCommon.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteAnimationGroupDefinition.hpp"
#include <Engine/Audio/AudioSystem.hpp>

enum Faction
{
	NEUTRAL,
	MARINE,
	DEMON,
	COUNT
};
enum BillboardType
{
	NONE,
	ALIGNED,
	FACING
};

class WeaponDefinition;
class ActorDefinition
{
public:
	bool LoadFromXmlElement( const XmlElement& element );

	const SpriteAnimationGroupDefinition* GetDefaultAnimationGroup() const;
	const SpriteAnimationGroupDefinition* GetAnimationGroup( const std::string& name ) const;

	std::string m_name;
	std::vector<const WeaponDefinition*> m_weaponDefinitions;

	// Physics
	float m_physicsRadius = -1.0f;
	float m_physicsHeight = -1.0f;
	float m_walkSpeed = 0.0f;
	float m_runSpeed = 0.0f;
	float m_drag = 0.0f;
	float m_turnSpeed = 0.0f;
	bool m_flying = false;
	bool m_simulated = false;
	bool m_collidesWithWorld = true;
	bool m_collidesWithActors = true;

	// Possession
	bool m_canBePossessed = true;
	float m_eyeHeight = 1.75f;
	float m_cameraFOVDegrees = 60.0f;

	// AI
	bool m_aiEnabled = false;
	float m_sightRadius = 64.0f;
	float m_sightAngle = 180.0f;
	FloatRange m_meleeDamage = FloatRange( 1.0f, 2.0f );
	float m_meleeDelay = 1.0f;
	float m_meleeRange = 0.5f;

	// Combat
	float m_health = -1.0f;
	Faction m_faction = Faction::NEUTRAL;
	float m_corpseLifetime = 0.0f;
	bool m_dieOnCollide = false;
	FloatRange m_damageOnCollide = FloatRange( 0.0f, 0.0f );
	float m_impulseOnCollide = 0.0f;

	// Visuals
	Vec2 m_spriteSize = Vec2( 1.0f, 1.0f );
	Vec2 m_spritePivot = Vec2( 0.5f, 0.0f );
	BillboardType m_billboardType = BillboardType::NONE;
	bool m_renderDepth = true;
	bool m_renderLit = true;
	bool m_renderRounded = true;
	bool m_isVisible = true;
	bool m_dieOnSpawn = false;
	SoundID m_hurtSound = MISSING_SOUND_ID;
	SoundID m_deathSound = MISSING_SOUND_ID;
	SoundID m_attackSound = MISSING_SOUND_ID;
	std::vector<SpriteAnimationGroupDefinition> m_spriteAnimationGroupDefinitions;

	static void InitializeDefinitions( const char* path );
	static void ClearDefinitions();
	static const ActorDefinition* GetByName( const std::string& name );
	static std::vector<ActorDefinition*> s_definitions;
};

