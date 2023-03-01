#include "Game/GameCommon.hpp"
#include "Engine/Math/FloatRange.hpp"
#include <string>
#include <vector>
#include "Engine/Core/SpriteAnimDefinition.hpp"
#include "Engine/Audio/AudioSystem.hpp"

class ActorDefinition;

class WeaponDefinition
{
public:
	WeaponDefinition();
	bool LoadFromXmlElement(const XmlElement& element);

	std::string m_name;
	float m_refireTime = 0.5f;

	int m_numRays = 0;
	float m_rayCone = 0.0f;
	float m_rayRange = 40.0f;
	FloatRange m_rayDamage = FloatRange(0.0f, 1.0f);
	float m_rayImpulse = 4.0f;

	int m_numProjectiles = 0;
	const ActorDefinition* m_projectileActorDefinition = nullptr;
	float m_projectileCone = 1.0f;
	float m_projectileSpeed = 40.0f;

	SpriteAnimDefinition* m_idleAnimationDefinition = nullptr;
	SpriteAnimDefinition* m_attackAnimationDefinition = nullptr;

	Shader* m_shader = nullptr;
	Shader* m_fireShader = nullptr;
	Texture* m_hudBaseTexture = nullptr;
	SpriteSheet* m_spriteSheet = nullptr;
	Vec2 m_spriteSize = Vec2(1.0f, 1.0f);
	Vec2 m_spritePivot = Vec2(0.5f, 0.0f);
	Texture* m_reticleTexture = nullptr;
	Texture* m_fireNoiseTexture = nullptr;
	Texture* m_fireAlphaTexture = nullptr;
	Texture* m_fireDiffuseTexture = nullptr;
	Texture* m_blueFireDiffuseTexture = nullptr;
	Vec2 m_reticleSize = Vec2(1.0f, 1.0f);

	std::string m_fireSoundName;
	SoundID m_fireSound = MISSING_SOUND_ID;
	static void InitializeDefinitions(const char* path);
	static void ClearDefinitions();
	static const WeaponDefinition* GetByName(const std::string& name);
	static std::vector<WeaponDefinition*> s_definitions;
};

