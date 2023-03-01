#include "Actor.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Game/Weapon.hpp"
#include "Game/Controller.hpp"
#include "Game/AI.hpp"
#include "Game/Map.hpp"
#include "Game/SpawnInfo.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/Controller.hpp"
#include <Engine/Math/VertexUtils.hpp>
#include <Engine/Math/RandomNumberGenerator.hpp>
#include "Engine/Math/Mat44.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"

extern Renderer* g_theRenderer;
extern AudioSystem* g_theAudioSystem;

Actor::~Actor()
{
}
Actor::Actor(Map* map, const SpawnInfo& spawnInfo)
{
	m_map = map;
	m_position = spawnInfo.m_position;
	m_orientationDegrees = spawnInfo.m_orientation;
	m_velocity = spawnInfo.m_velocity;
}
Actor::Actor(Game* game, Vec3 position, EulerAngles orientationDegrees, EulerAngles angularVelocity, float height, float radius)
{
	m_game = game;
	m_position = position;
	m_orientationDegrees = orientationDegrees;
	m_angularVelocity = angularVelocity;
	m_physicsHeight = height;
	m_physicsRadius = radius;
	AddVertsForZCylinder(m_vertices, m_physicsHeight, m_physicsRadius, AABB2::ZERO_TO_ONE, Rgba8::WHITE);
	AddVertsForZCylinder(m_wireFrameVertices, m_physicsHeight + 0.001f, m_physicsRadius + 0.001f, AABB2::ZERO_TO_ONE, Rgba8::WHITE);
}
void Actor::AddVertsForActor()
{
	Vec2 actorPivot = m_definition->m_spritePivot;
	Vec2 actorSize = m_definition->m_spriteSize;
	if (m_definition->m_name == "Marine" || m_definition->m_name == "FlyingDemon")
	{
		m_health = m_definition->m_health;
	}
	m_currentAnimation = m_definition->GetDefaultAnimationGroup();
	m_animationStopwatch.Start(&m_animationClock, m_currentAnimation->GetDuration());

	actorQuad.m_mins.x = 0.0f;
	actorQuad.m_maxs.x = actorSize.x /2.0f;
	actorQuad.m_mins.y = 0;
	actorQuad.m_maxs.y = actorSize.y;
	if (m_definition->m_name == "BloodSplatter" || m_definition->m_name == "BulletHit" || m_definition->m_name == "PlasmaProjectile")
	{
		actorQuad.m_mins.x = -0.5f * actorSize.x;
		actorQuad.m_maxs.x = 0.5f * actorSize.x;
		actorQuad.m_mins.y = -0.5f * actorSize.y;
		actorQuad.m_maxs.y = 0.5f * actorSize.y;
	}
	else if (m_definition->m_name == "Fire")
	{
		actorQuad.m_mins.x = -0.2f * actorSize.x;
		actorQuad.m_maxs.x = 0.2f * actorSize.x;
		actorQuad.m_mins.y = 0.2f * actorSize.y;
		actorQuad.m_maxs.y = -0.2f * actorSize.y;
	}
}
void Actor::CreateWeaponsFromInventory()
{
	for (int i = 0; i < m_definition->m_weaponDefinitions.size(); i++)
	{
		Weapon* weapon = new Weapon(m_definition->m_weaponDefinitions[i]);
		m_weapons.push_back(weapon);
	}
}

void Actor::Update(float deltaSeconds)
{
	if (m_isDead)
	{
		UpdatePhysics(deltaSeconds);
		return;
	}
	if (m_controller != nullptr)
	{
		m_controller->Update(deltaSeconds);
	}
	UpdatePhysics(deltaSeconds);
	UpdateAnimations(deltaSeconds);
	CheckForActorCollisionsAgainstOtherActors();
	CheckForActorCollisionsAgainstWall();
	if ((m_definition->m_dieOnSpawn  || m_health <= 0.0f) && !m_isDead)
	{
		Die();
	}

}
void Actor::UpdatePhysics(float deltaSeconds)
{
	if (m_isDead)
	{
		if (m_definition->m_name == "FlyingDemon")
		{
			m_velocity = Vec3::ZERO;
			m_velocity.z = -0.8f;
			m_position += m_velocity * deltaSeconds;
		}
		return;
	}
	if(!m_definition->m_dieOnCollide)
	{
		AddForce(-m_definition->m_drag * m_velocity);
		m_velocity += m_acceleration * deltaSeconds;
	}
	if (m_definition->m_name == "FlyingDemon" && m_isDead)
	{
		m_velocity.z = 0.5f;
	}
	m_position += m_velocity * deltaSeconds;

	ClampActorRotation();
	if (m_definition->m_dieOnCollide)
	{
		if (m_position.z < 0.0f ||  m_position.z > 1.0f)
		{
			Die();
		}
	}
	m_acceleration = Vec3::ZERO;
}
void Actor::UpdateAnimations(float deltaSeconds)
{
	UNUSED((void)deltaSeconds);
	if (m_currentAnimation)
	{
		if (m_currentAnimation->m_scaleBySpeed)
		{
			float dilation = m_velocity.GetLength() / m_definition->m_runSpeed;
			m_animationClock.SetTimeDilation(dilation);
		}
		else
		{
			m_animationClock.SetTimeDilation(1.f);
		}

		if (m_animationStopwatch.HasDurationElapsed())
		{
			PlayAnimationGroupByName(m_definition->GetDefaultAnimationGroup()->m_name);
		}
	}
}
void Actor::Render() const
{
	int currentPlayerIndex = m_map->m_game->m_currentMap->m_currentPlayerRendering;
	Player* isplayer = dynamic_cast<Player*> (m_controller);
	Controller* player = m_map->m_game->m_players[currentPlayerIndex];
	Player* playerController = m_map->m_game->m_players[currentPlayerIndex];
	//if (m_definition->m_name == "PlasmaProjectile" || )
	//{
	//	player = m_owner->m_controller;
	//}
	if (isplayer)
	{
		if (isplayer == player)
		{
			return;
		}
	}
	float modelColor[4];
	if (m_isDead)
	{
		Rgba8::GREY.GetAsFloats(modelColor);
	}
	else
	{
		Rgba8::WHITE.GetAsFloats(modelColor);
	}
	std::vector<Vertex_PNCU> actorVerts;
	std::vector<Vertex_PCU> projectileVerts;
	//----------------GETTING ANIMATION UVS -------------------------------
	Vec3 ibasis, jbasis, kbasis;
	if (player == nullptr)
	{
		return;
	}
	ibasis = playerController->m_position - m_position;
	ibasis.z = 0.0f;
	ibasis = ibasis.GetNormalized();
	jbasis = Vec3(-ibasis.y, ibasis.x, 0.0f);
	kbasis = Vec3(0.0f, 0.0f, 1.0f);
	//----------------DRAWING QUAD-----------------------------------------
	Vec3 topleft, bottomleft, topRight, bottomRight;
	topleft = (jbasis * actorQuad.m_maxs.x) + (kbasis * actorQuad.m_mins.y);
	bottomleft = (jbasis * actorQuad.m_maxs.x) + (kbasis * actorQuad.m_maxs.y);
	topRight = (-jbasis * actorQuad.m_maxs.x) + (kbasis * actorQuad.m_mins.y);
	bottomRight = (-jbasis * actorQuad.m_maxs.x) + (kbasis * actorQuad.m_maxs.y);

	Vec2 animationFwdNormal;
	Vec2 animationi = Vec2(-ibasis).GetNormalized();
	Vec2 animationj = Vec2(-animationi.y, animationi.x);
	Vec2 playerForward = Vec2(GetForward()).GetNormalized();
	animationFwdNormal.x = DotProduct2D(animationi, playerForward);
	animationFwdNormal.y = DotProduct2D(animationj, playerForward);
	animationFwdNormal = animationFwdNormal.GetNormalized();
	AABB2 actorQuadUVs;
	SpriteAnimDefinition spriteAnimDef = m_currentAnimation->GetAnimationForDirection(Vec3(animationFwdNormal));
	SpriteDefinition spriteDef = spriteAnimDef.GetSpriteDefAtTime((float)m_animationStopwatch.GetElapsedTime());
	//if (m_currentAnimation->m_name == "Walk" && m_velocity.GetLength() == 0.0f)
	//{
	//	actorQuadUVs = m_currentAnimation->GetAnimationForDirection(Vec3(animationFwdNormal)).GetSpriteDefAtTime(1.0f).GetUVs();
	//}
	//else
	//{
	//	actorQuadUVs = spriteDef.GetUVs();
	//}
	actorQuadUVs = spriteDef.GetUVs();
	if (m_definition->m_billboardType == BillboardType::FACING)
	{
		if (m_definition->m_renderRounded)
		{
			AddVertsForRoundedQuad3D(actorVerts, topleft, bottomleft, bottomRight, topRight, Rgba8::WHITE, actorQuadUVs);
		}
		else
		{
			AddVertsForQuadPNCU3D(actorVerts, topleft, bottomleft, bottomRight, topRight, Rgba8::WHITE, actorQuadUVs);
		}
		
		Mat44 modelMatrix = Mat44::CreateTranslation3D(m_position);
		TransformTranslateVertexArrayUsingMatrix3D((int)actorVerts.size(), actorVerts, modelMatrix);

		g_theRenderer->SetModalMatrix(Mat44());
		g_theRenderer->SetModalColor(modelColor);
		g_theRenderer->BindShader(m_definition->GetDefaultAnimationGroup()->m_shader);
		g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
		if (m_definition->m_renderDepth)
		{
			g_theRenderer->SetDepthStencilState(DepthTest::LESSEQUAL, true);
		}
		else
		{
			g_theRenderer->SetDepthStencilState(DepthTest::ALWAYS, false);
		}
		g_theRenderer->SetSamplerMode(SamplerMode::BILINEARWRAP);
		g_theRenderer->BindTexture(&m_definition->GetDefaultAnimationGroup()->m_spriteSheet->GetTexture());
		g_theRenderer->DrawVertexArray((int)actorVerts.size(), actorVerts);
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->BindShader(nullptr);
	}
	if (m_definition->m_billboardType == BillboardType::ALIGNED)
	{ 
		if (m_definition->m_name == "Fire" && !m_isDead)
		{
			AddVertsForSphere3D(projectileVerts, 0.2f, Vec3::ZERO, AABB2::ZERO_TO_ONE, Rgba8::WHITE);
			float rotationDegrees = animationi.GetOrientationDegrees();
			TransformVertexArray3D((int)projectileVerts.size(), projectileVerts, 1.0f, rotationDegrees, Vec3::ZERO);
			//AddVertsForQuad3D(projectileVerts, topleft, bottomleft, bottomRight, topRight, Rgba8::WHITE, AABB2::ZERO_TO_ONE);
		}
		else if(m_definition->m_name != "Fire")
		{
			AddVertsForQuad3D(projectileVerts, topleft, bottomleft, bottomRight, topRight, Rgba8::WHITE, actorQuadUVs);
		}
		
		Mat44 modelMatrix = Mat44::CreateTranslation3D(m_position);
		if (m_definition->m_name == "Fire")
		{
			Mat44 rotationMatrix = m_orientationDegrees.GetAsMatrix_XFwd_YLeft_ZUp();
			modelMatrix.Append(rotationMatrix);
		}
		TransformTranslateVertexArrayUsingMatrix3D((int)projectileVerts.size(), projectileVerts, modelMatrix);

		g_theRenderer->SetModalMatrix(Mat44());
		g_theRenderer->SetModalColor(modelColor);
		g_theRenderer->BindShader(m_definition->GetDefaultAnimationGroup()->m_shader);
		//g_theRenderer->BindShader(g_theRenderer->GetDefaultShader());
		g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
		if (m_definition->m_renderDepth)
		{
			g_theRenderer->SetDepthStencilState(DepthTest::LESSEQUAL, true);
		}
		else
		{
			g_theRenderer->SetDepthStencilState(DepthTest::ALWAYS, false);
		}
		if (m_definition->m_name == "Fire" && !m_owner->m_isDead && m_owner != nullptr)
		{
			g_theRenderer->SetSamplerMode(SamplerMode::BILINEARWRAP);
			g_theRenderer->BindTexture(nullptr);
			if (m_owner->m_definition->m_name != "Fire")
			{
				g_theRenderer->BindShader(m_owner->GetWeapon(2)->m_definition->m_fireShader);
				if (m_owner->m_definition->m_name == "FlyingDemon")
				{
					g_theRenderer->BindTexture(m_owner->GetWeapon(2)->m_definition->m_blueFireDiffuseTexture, 0);
					g_theRenderer->BindTexture(m_owner->GetWeapon(2)->m_definition->m_fireNoiseTexture, 1);
					g_theRenderer->BindTexture(m_owner->GetWeapon(2)->m_definition->m_fireAlphaTexture, 2);
				}
				else
				{
					g_theRenderer->BindTexture(m_owner->GetWeapon(2)->m_definition->m_fireDiffuseTexture, 0);
					g_theRenderer->BindTexture(m_owner->GetWeapon(2)->m_definition->m_fireNoiseTexture, 1);
					g_theRenderer->BindTexture(m_owner->GetWeapon(2)->m_definition->m_fireAlphaTexture, 2);
				}
			}
		
			g_theRenderer->SetRasterizerState(CullMode::NONE, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
			g_theRenderer->DrawVertexArray((int)projectileVerts.size(), projectileVerts);
		}
		else
		{
			g_theRenderer->SetSamplerMode(SamplerMode::BILINEARWRAP);
			g_theRenderer->BindTexture(&m_definition->GetDefaultAnimationGroup()->m_spriteSheet->GetTexture());
			g_theRenderer->DrawVertexArray((int)projectileVerts.size(), projectileVerts);
		}
		
	}
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader(nullptr);
	actorVerts.clear();
	projectileVerts.clear();
}

Vec3 Actor::GetForward() const
{
	Mat44 modalMatrix = GetModalMatrix();
	return modalMatrix.GetIBasis3D();
}
Vec3 Actor::GetJForward() const
{
	Mat44 modalMatrix = GetModalMatrix();
	return modalMatrix.GetJBasis3D();
}
void Actor::ClampActorRotation()
{
	m_orientationDegrees.m_pitchDegrees = Clamp(m_orientationDegrees.m_pitchDegrees, -85.0f, 85.0f);
	m_orientationDegrees.m_rollDegrees = Clamp(m_orientationDegrees.m_rollDegrees, -45.0f, 45.0f);
}

void Actor::Damage(float damage)
{
	m_health -= damage;
	if (m_health <= 0.0f) { m_health = 0.0f; };
	PlayAnimationGroupByName("Pain");
	if (m_definition->m_hurtSound != MISSING_SOUND_ID)
	{
		g_theAudioSystem->StartSoundAt(m_definition->m_hurtSound, m_position);
	}
}
void Actor::Die()
{
	if (!m_isDead)
	{
		PlayAnimationGroupByName("Death");
		m_lifetimeStopwatch.Start(m_definition->m_corpseLifetime);
		m_isDead = true;
		if (m_definition->m_deathSound != MISSING_SOUND_ID)
		{
			g_theAudioSystem->StartSoundAt(m_definition->m_deathSound, m_position);
		}
	}
}
void Actor::AddForce(const Vec3& force)
{
	m_acceleration += force ;
}
void Actor::AddImpulse(const Vec3& impulse)
{
	m_velocity += impulse;
}
void Actor::OnCollide(Actor* other)
{
	RandomNumberGenerator rng;
	Vec3 otherActorPosition = other->m_position;
	const ActorDefinition* actorDef  = other->m_definition;
	float randomDamage = rng.GetRandomFloatInRange(actorDef->m_damageOnCollide.m_min, actorDef->m_damageOnCollide.m_max);
	if (!m_definition->m_dieOnCollide)
	{
		Damage(randomDamage);
		Vec2 impulse = (Vec2(m_position) - Vec2(otherActorPosition)).GetNormalized();
		AddImpulse(Vec3(impulse) * other->m_definition->m_impulseOnCollide);	
	}
	if (other->m_definition->m_dieOnCollide)
	{
		other->Die();
	}
}
void Actor::OnPossessed(Controller* controller)
{
	m_controller = controller;
	
}
void Actor::OnUnpossessed(Controller* controller)
{
	UNUSED((void)controller);
	m_controller = nullptr;
}
void Actor::MoveInDirection(Vec3 direction, float speed)
{
	AddForce(direction * speed * m_definition->m_drag);
}
void Actor::TurnInDirection(EulerAngles direction, float deltaSeconds)
{
	float turnSpeed = m_definition->m_turnSpeed * deltaSeconds;
	m_orientationDegrees.m_yawDegrees = GetTurnedTowardDegrees(m_orientationDegrees.m_yawDegrees,direction.m_yawDegrees, turnSpeed);
	m_orientationDegrees.m_rollDegrees = GetTurnedTowardDegrees(m_orientationDegrees.m_rollDegrees,direction.m_rollDegrees, turnSpeed);
	m_orientationDegrees.m_pitchDegrees = GetTurnedTowardDegrees(m_orientationDegrees.m_pitchDegrees,direction.m_pitchDegrees, turnSpeed);
}

Weapon* Actor::GetEquippedWeapon()
{
	if (m_weapons.size() == 0 || m_equippedWeaponIndex == -1 || m_equippedWeaponIndex >= m_weapons.size())
	{
		return nullptr;
	}
	return m_weapons[m_equippedWeaponIndex];
}
void Actor::EquipWeapon(int weaponIndex)
{
	if (m_weapons.size() == 0 || weaponIndex == -1 || weaponIndex > m_weapons.size())
	{
		return;
	}
	m_equippedWeaponIndex = weaponIndex;
}
void Actor::EquipNextWeapon()
{
	m_equippedWeaponIndex++;
	if (m_equippedWeaponIndex >= m_weapons.size())
	{
		m_equippedWeaponIndex = 0;
	}
}
Weapon* Actor::GetWeapon(int weaponIndex)
{
	if (m_weapons.size() == 0 || weaponIndex == -1 || weaponIndex > m_weapons.size())
	{
		return nullptr;
	}
	else
	{
		return m_weapons[weaponIndex];
	}
}
void Actor::EquipPreviousWeapon()
{
	m_equippedWeaponIndex--;
	if (m_equippedWeaponIndex < 0)
	{
		m_equippedWeaponIndex = (int)m_weapons.size() - 1 ;
	}
}
void Actor::Attack()
{
	PlayAnimationGroupByName("Attack");
	if (m_definition->m_attackSound != MISSING_SOUND_ID)
	{
		g_theAudioSystem->StartSoundAt(m_definition->m_attackSound, m_position);
	}
}

void Actor::PlayAnimationGroupByName(std::string name)
{
	m_animationClock.SetTimeDilation(1.0f);
	m_currentAnimation = m_definition->GetAnimationGroup(name);
	if (m_currentAnimation == nullptr)
	{
		m_currentAnimation = m_definition->GetDefaultAnimationGroup();
	}
	float animationDuration = m_currentAnimation->GetDuration();
	m_animationStopwatch.Start(&m_animationClock,animationDuration);
}
Mat44 Actor::GetModalMatrix() const
{
	Mat44 translationMatrix = Mat44::CreateTranslation3D(m_position);
	Mat44 rotationMatrix = m_orientationDegrees.GetAsMatrix_XFwd_YLeft_ZUp();
	translationMatrix.Append(rotationMatrix);
	return translationMatrix;
}
void Actor::CheckForActorCollisionsAgainstWall()
{
	AABB2 bounds;
	Vec2 actorPosition2D = Vec2(m_position);
	IntVec2 actorPositionInt2D = IntVec2(actorPosition2D);
	IntVec2 left = actorPositionInt2D + IntVec2(0, 1);
	IntVec2 back = actorPositionInt2D + IntVec2(-1, 0);
	IntVec2 front = actorPositionInt2D + IntVec2(1, 0);
	IntVec2 right = actorPositionInt2D + IntVec2(0, -1);
	IntVec2 frontRight = actorPositionInt2D + front + right;
	IntVec2 frontLeft = actorPositionInt2D + front + left;
	IntVec2 backLeft = actorPositionInt2D + back + left;
	IntVec2 backRight = actorPositionInt2D + back + right;
	//----------------CHECK ALL 4 BOXES (TOP,LEFT,RIGHT,BOTTOM)--------------------------
	bool wasPushed;
	if (m_map->GetTileAtPosition(left)->IsSolid())
	{
		bounds.m_mins = Vec2(m_map->GetTileAtPosition(left)->m_bounds.m_mins);
		bounds.m_maxs = Vec2(m_map->GetTileAtPosition(left)->m_bounds.m_maxs);
		wasPushed = PushDiscOutOfAABB2D(actorPosition2D, m_physicsRadius, bounds);
		if (m_definition->m_dieOnCollide && !m_isDead && wasPushed)
		{
			Die();
			return;
		}
	}
	if (m_map->GetTileAtPosition(back)->IsSolid())
	{
		bounds.m_mins = Vec2(m_map->GetTileAtPosition(back)->m_bounds.m_mins);
		bounds.m_maxs = Vec2(m_map->GetTileAtPosition(back)->m_bounds.m_maxs);
		wasPushed = PushDiscOutOfAABB2D(actorPosition2D, m_physicsRadius, bounds);
		if (m_definition->m_dieOnCollide && !m_isDead && wasPushed)
		{
			Die();
			return;
		}
	}
	if (m_map->GetTileAtPosition(front)->IsSolid())
	{
		bounds.m_mins = Vec2(m_map->GetTileAtPosition(front)->m_bounds.m_mins);
		bounds.m_maxs = Vec2(m_map->GetTileAtPosition(front)->m_bounds.m_maxs);
		wasPushed = PushDiscOutOfAABB2D(actorPosition2D, m_physicsRadius, bounds);
		if (m_definition->m_dieOnCollide && !m_isDead && wasPushed)
		{
			Die();
			return;
		}
	}
	if (m_map->GetTileAtPosition(right)->IsSolid())
	{
		bounds.m_mins = Vec2(m_map->GetTileAtPosition(right)->m_bounds.m_mins);
		bounds.m_maxs = Vec2(m_map->GetTileAtPosition(right)->m_bounds.m_maxs);
		wasPushed = PushDiscOutOfAABB2D(actorPosition2D, m_physicsRadius, bounds);
		if (m_definition->m_dieOnCollide && !m_isDead && wasPushed)
		{
			Die();
			return;
		}
	}
	if (m_map->GetTileAtPosition(frontRight)->IsSolid())
	{
	
		bounds.m_mins = Vec2(m_map->GetTileAtPosition(frontRight)->m_bounds.m_mins);
		bounds.m_maxs = Vec2(m_map->GetTileAtPosition(frontRight)->m_bounds.m_maxs);
		wasPushed = PushDiscOutOfAABB2D(actorPosition2D, m_physicsRadius, bounds);
		if (m_definition->m_dieOnCollide && !m_isDead && wasPushed)
		{
			Die();
			return;
		}
	}
	if (m_map->GetTileAtPosition(frontLeft)->IsSolid())
	{
		bounds.m_mins = Vec2(m_map->GetTileAtPosition(frontLeft)->m_bounds.m_mins);
		bounds.m_maxs = Vec2(m_map->GetTileAtPosition(frontLeft)->m_bounds.m_maxs);
		wasPushed = PushDiscOutOfAABB2D(actorPosition2D, m_physicsRadius, bounds);
		if (m_definition->m_dieOnCollide && !m_isDead && wasPushed)
		{
			Die();
			return;
		}
	}
	if (m_map->GetTileAtPosition(backLeft)->IsSolid())
	{

		bounds.m_mins = Vec2(m_map->GetTileAtPosition(backLeft)->m_bounds.m_mins);
		bounds.m_maxs = Vec2(m_map->GetTileAtPosition(backLeft)->m_bounds.m_maxs);
		wasPushed = PushDiscOutOfAABB2D(actorPosition2D, m_physicsRadius, bounds);
		if (m_definition->m_dieOnCollide && !m_isDead && wasPushed)
		{
			Die();
			return;
		}
	}
	if (m_map->GetTileAtPosition(backRight)->IsSolid())
	{
		bounds.m_mins = Vec2(m_map->GetTileAtPosition(backRight)->m_bounds.m_mins);
		bounds.m_maxs = Vec2(m_map->GetTileAtPosition(backRight)->m_bounds.m_maxs);
		wasPushed = PushDiscOutOfAABB2D(actorPosition2D, m_physicsRadius, bounds);
		if (m_definition->m_dieOnCollide && !m_isDead && wasPushed)
		{
			Die();
			return;
		}
	}

	/*PushDiscOutOfAABB2D(actorPosition2D, m_physicsRadius, AABB2(actorPositionInt2D, back));
	PushDiscOutOfAABB2D(actorPosition2D, m_physicsRadius, AABB2(actorPositionInt2D, front));
	PushDiscOutOfAABB2D(actorPosition2D, m_physicsRadius, AABB2(actorPositionInt2D, right));
	PushDiscOutOfAABB2D(actorPosition2D, m_physicsRadius, AABB2(actorPositionInt2D, frontRight));
	PushDiscOutOfAABB2D(actorPosition2D, m_physicsRadius, AABB2(actorPositionInt2D, frontLeft));
	PushDiscOutOfAABB2D(actorPosition2D, m_physicsRadius, AABB2(actorPositionInt2D, backLeft));
	PushDiscOutOfAABB2D(actorPosition2D, m_physicsRadius, AABB2(actorPositionInt2D, backRight));*/
	m_position = Vec3(actorPosition2D.x, actorPosition2D.y, m_position.z);
	
}
void Actor::CheckForActorCollisionsAgainstOtherActors()
{
	//RandomNumberGenerator rng;
	//Vec3 impulse;
	//Vec2 actorPosition;
	//Vec2 myPosition = Vec2(m_position);
	//for (int i = 0; i < (int)m_map->m_actors.size(); i++)
	//{
	//	if (m_map->m_actors[i] && m_map->m_actors[i] != this)
	//	{
	//		if (m_definition->m_name == "PlasmaProjectile" && !m_isDead && m_owner != m_map->m_actors[i])
	//		{
	//			actorPosition = Vec2(m_map->m_actors[i]->m_position);
	//			if (DoDiscsOverlap(actorPosition, m_map->m_actors[i]->m_physicsRadius, myPosition, m_physicsRadius))
	//			{
	//				if (m_map->m_actors[i]->m_definition->m_faction == Faction::DEMON)
	//				{
	//					float randomDamage = rng.GetRandomFloatInRange(m_definition->m_damageOnCollide.m_min, m_definition->m_damageOnCollide.m_max);
	//					m_map->m_actors[i]->Damage(randomDamage);
	//					impulse = (Vec3(actorPosition) - Vec3(myPosition)).GetNormalized();
	//					m_map->m_actors[i]->AddImpulse(impulse * m_definition->m_impulseOnCollide);
	//				}
	//				Die();
	//			}
	//		}
	//		else if (m_map->m_actors[i]->m_definition->m_name == "PlasmaProjectile")
	//		{

	//		}
	//		else if (m_definition->m_dieOnCollide && !m_isDead && !m_map->m_actors[i]->m_definition->m_dieOnCollide)
	//		{
	//			actorPosition = Vec2(m_map->m_actors[i]->m_position);
	//			if (DoDiscsOverlap(actorPosition, m_map->m_actors[i]->m_physicsRadius, myPosition, m_physicsRadius))
	//			{
	//				if (m_map->m_actors[i]->m_definition->m_faction == Faction::DEMON)
	//				{
	//					float randomDamage = rng.GetRandomFloatInRange(m_definition->m_damageOnCollide.m_min, m_definition->m_damageOnCollide.m_max);
	//					m_map->m_actors[i]->Damage(randomDamage);
	//					impulse = (Vec3(actorPosition) - Vec3(myPosition)).GetNormalized();
	//					m_map->m_actors[i]->AddImpulse(impulse * m_definition->m_impulseOnCollide );
	//				}
	//				Die();
	//			}
	//		
	//		}
	//		else if (m_map->m_actors[i] != this && !(m_definition->m_dieOnCollide && m_map->m_actors[i]->m_definition->m_dieOnCollide) 
	//			&& !(m_definition->m_dieOnCollide && (m_map->m_actors[i]->m_controller == m_map->GetPlayer(0))) && !((m_controller == m_map->GetPlayer(0)) && m_map->m_actors[i]->m_definition->m_dieOnCollide))
	//		{
	//			actorPosition = Vec2(m_map->m_actors[i]->m_position);
	//			PushDiscsOutOfEachOther2D(actorPosition, m_map->m_actors[i]->m_physicsRadius, myPosition, m_physicsRadius);
	//			m_map->m_actors[i]->m_position.x = actorPosition.x;
	//			m_map->m_actors[i]->m_position.y = actorPosition.y;
	//			m_position.x = myPosition.x;
	//			m_position.y = myPosition.y;
	//		}
	//		else if (m_map->m_actors[i] != this && !(m_definition->m_dieOnCollide && m_map->m_actors[i]->m_definition->m_dieOnCollide)
	//			&& !(m_definition->m_dieOnCollide && (m_map->m_actors[i]->m_controller == m_map->GetPlayer(1))) && !((m_controller == m_map->GetPlayer(1)) && m_map->m_actors[i]->m_definition->m_dieOnCollide))
	//		{
	//			actorPosition = Vec2(m_map->m_actors[i]->m_position);
	//			PushDiscsOutOfEachOther2D(actorPosition, m_map->m_actors[i]->m_physicsRadius, myPosition, m_physicsRadius);
	//			m_map->m_actors[i]->m_position.x = actorPosition.x;
	//			m_map->m_actors[i]->m_position.y = actorPosition.y;
	//			m_position.x = myPosition.x;
	//			m_position.y = myPosition.y;
	//		}
	//	}
	//
	//}
	RandomNumberGenerator rng;
	Vec3 impulse;
	Vec2 actorPosition;
	Vec2 myPosition = Vec2(m_position);
	for (int i = 0; i < (int)m_map->m_actors.size(); i++)
	{
		Actor* Collidingactor = m_map->m_actors[i];
		if (Collidingactor == this || Collidingactor == nullptr)
		{
			continue;
		}
		else if (Collidingactor->m_owner == this || m_owner == Collidingactor || m_isDead || Collidingactor->m_isDead)
		{
			continue;
		}
		actorPosition = Vec2(Collidingactor->m_position);
		
		if (DoDiscsOverlap(actorPosition, Collidingactor->m_physicsRadius, myPosition, m_physicsRadius))
		{
			PushDiscsOutOfEachOther2D(actorPosition, Collidingactor->m_physicsRadius, myPosition, m_physicsRadius);
			Collidingactor->m_position.x = actorPosition.x;
			Collidingactor->m_position.y = actorPosition.y;
			m_position.x = myPosition.x;
			m_position.y = myPosition.y;
			if (m_definition->m_name == "PlasmaProjectile" || Collidingactor->m_definition->m_name == "PlasmaProjectile" || 
				m_definition->m_name == "Fire" || Collidingactor->m_definition->m_name == "Fire")
			{
				OnCollide(Collidingactor);
			}
	
		}
	}

}
