#include "Player.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Input/XBoxController.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Game/Weapon.hpp"
#include <Engine/Math/VertexUtils.hpp>
#include <String>
#include <Engine/Core/ErrorWarningAssert.hpp>


extern AudioSystem* g_theAudioSystem;
extern Renderer* g_theRenderer;
extern InputSystem* g_theInputSystem;
Player::Player() : Controller()
{
}

Player::~Player()
{
}

Player::Player(Game* game, Vec3 position, EulerAngles orientationDegrees, EulerAngles angularVelocity)
{
	m_game = game;
	m_position = position;
	m_orientationDegrees = orientationDegrees;
	m_angularVelocity = angularVelocity;
}

void Player::Render() const
{
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Images/SquirrelFixedFont");
	g_theRenderer->SetDepthStencilState(DepthTest::ALWAYS, false);
	g_theRenderer->SetRasterizerState(CullMode::NONE, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	g_theRenderer->SetSamplerMode(SamplerMode::POINTCLAMP);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->BeginCamera(m_screenCamera);
	Actor* actor = GetActor();
	if (actor)
	{
		Weapon* weapon = actor->GetEquippedWeapon();
		g_theRenderer->BindShader(weapon->m_definition->m_shader);
		g_theRenderer->BindTexture(weapon->m_definition->m_hudBaseTexture);
		g_theRenderer->DrawVertexArray((int)m_hudBox.size(), m_hudBox.data());
		g_theRenderer->BindTexture(nullptr);

		g_theRenderer->BindTexture(&weapon->m_definition->m_spriteSheet->GetTexture());
		g_theRenderer->DrawVertexArray((int)m_weaponBox.size(), m_weaponBox.data());

		int playerHealth = (int)actor->m_health;
		std::string health = std::to_string(playerHealth);
		std::string kills = std::to_string(m_kills);

		std::vector<Vertex_PCU> m_healthverts;
		font->AddVertsForTextInBox2D(m_healthverts, m_hudboxBounds, 60.0f, health, Rgba8::WHITE,
			0.6f, Vec2(0.23f, 0.52f), TextBoxMode::SHRINK_TO_FIT, 999999, 5.0f);


		font->AddVertsForTextInBox2D(m_healthverts, m_hudboxBounds, 60.0f, kills, Rgba8::WHITE,
			0.6f, Vec2(0.38f, 0.52f), TextBoxMode::SHRINK_TO_FIT, 999999, 5.0f);
		if (actor->GetEquippedWeapon()->m_definition->m_name == "Pistol")
		{
			g_theRenderer->BindTexture(weapon->m_definition->m_reticleTexture);
			g_theRenderer->DrawVertexArray((int)m_reticleBox.size(), m_reticleBox);
		}
		if (actor->GetEquippedWeapon()->m_definition->m_name == "Firethrower")
		{
			g_theRenderer->SetSamplerMode(SamplerMode::BILINEARWRAP);
	/*		g_theRenderer->SetSamplerMode(SamplerMode::BILINEARCLAMP, 1, m_samplerState2);*/
			g_theRenderer->BindShader(weapon->m_definition->m_fireShader);
			g_theRenderer->BindTexture(weapon->m_definition->m_fireDiffuseTexture,0 );
			g_theRenderer->BindTexture(weapon->m_definition->m_fireNoiseTexture,1 );
			g_theRenderer->BindTexture(weapon->m_definition->m_fireAlphaTexture,2 );
			g_theRenderer->ReleaseSampler(m_samplerState);
			g_theRenderer->ReleaseSampler(m_samplerState2);
			g_theRenderer->DrawVertexArray((int)m_fireBox.size(), m_fireBox);
		
		}
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(&font->GetTexture());
		g_theRenderer->DrawVertexArray((int)m_healthverts.size(), m_healthverts);
		g_theRenderer->BindTexture(nullptr);
		m_healthverts.clear();

		if (actor->m_isDead)
		{
			AddVertsForAABB2D(m_healthverts, m_playerScreenBounds, Rgba8::BLACKLOWALPHA);
		}
		g_theRenderer->DrawVertexArray((int)m_healthverts.size(), m_healthverts);
		g_theRenderer->BindShader(nullptr);
		m_healthverts.clear();
	}
	g_theRenderer->EndCamera(m_screenCamera);

}
void Player::Update(float deltaSeconds)
{
	/*std::string debugClockText = Stringf("%.2f, %.2f, %.2f \n", m_position.x, m_position.y);
	DebuggerPrintf(debugClockText.c_str());*/
	AABB2 bounds;
	AABB2 reticleBounds;
	bounds = m_screenCamera.GetCameraBounds();
	AABB2 weaponBounds = m_screenCamera.GetCameraBounds(); 
	Actor* actor = GetActor();

	if ( m_game->m_numberOfPlayersJoined == 1 && m_playerIndex == 0)
	{
		m_playerScreenBounds = bounds;
	}
	else if (m_game->m_numberOfPlayersJoined > 1 && m_playerIndex == 1)
	{
		m_playerScreenBounds.m_mins.y = bounds.m_mins.y;
		m_playerScreenBounds.m_maxs.y = bounds.m_mins.y + (bounds.m_maxs.y * 0.5f);
		m_kills = m_map->GetGame()->m_players[0]->m_deaths;
	}
	else if(m_game->m_numberOfPlayersJoined > 1 && m_playerIndex == 0)
	{
		m_playerScreenBounds.m_mins.y = bounds.m_maxs.y * 0.5f;
		m_playerScreenBounds.m_maxs.y = bounds.m_mins.y + (bounds.m_maxs.y);
		m_kills = m_map->GetGame()->m_players[1]->m_deaths;
	}
	m_playerScreenBounds.m_mins.x = bounds.m_mins.x;
	m_playerScreenBounds.m_maxs.x = bounds.m_maxs.x;
	SpriteAnimDefinition* currentAnim;
	if (actor)
	{
		Weapon* weapon = actor->GetEquippedWeapon();
		Vec2 reticleSize = weapon->m_definition->m_reticleSize;
		if (m_game->m_numberOfPlayersJoined > 1 && m_playerIndex == 1 || (m_game->m_numberOfPlayersJoined == 1 && m_playerIndex == 0))
		{
			if (m_game->m_numberOfPlayersJoined == 1)
			{
				weaponBounds.m_mins.y = bounds.m_mins.y + (bounds.m_maxs.y * 0.50f);
				weaponBounds.m_maxs.y = bounds.m_mins.y + (bounds.m_maxs.y * 0.15f);
			}
			else
			{
				weaponBounds.m_mins.y = bounds.m_mins.y + (bounds.m_maxs.y * 0.3f);
				weaponBounds.m_maxs.y = bounds.m_mins.y + (bounds.m_maxs.y * 0.15f);
			}
		}
		else
		{
			weaponBounds.m_mins.y = bounds.m_mins.y + (bounds.m_maxs.y * 0.3f) + bounds.m_maxs.y * 0.5f;
			weaponBounds.m_maxs.y = bounds.m_mins.y + (bounds.m_maxs.y * 0.15f) + bounds.m_maxs.y * 0.5f;
		}
		if (!weapon->m_refireStopwatch.HasDurationElapsed())
		{
			currentAnim = weapon->m_definition->m_attackAnimationDefinition;
		}
		else
		{
			currentAnim = weapon->m_definition->m_idleAnimationDefinition;
		}
		SpriteDefinition spriteDef = currentAnim->GetSpriteDefAtTime((float)weapon->m_refireStopwatch.GetElapsedTime());
		weaponBounds.m_mins.x = weaponBounds.m_maxs.x * 0.45f;
		weaponBounds.m_maxs.x = weaponBounds.m_maxs.x * 0.55f;
		AddVertsForAABB2D(m_weaponBox, weaponBounds, spriteDef.GetUVs(), Rgba8::WHITE);

		reticleBounds = AABB2(Vec2() - (reticleSize.x * 0.5f), reticleSize * 0.5f);
		if ((int)m_reticleBox.size() == 0)
		{
			Vec2 center = m_playerScreenBounds.GetCenter();
			reticleBounds.Translate(center);
			AddVertsForAABB2D(m_reticleBox, reticleBounds, AABB2::ZERO_TO_ONE, Rgba8::WHITE);
		}
		if ((int)m_hudBox.size() == 0)
		{
			if (m_game->m_numberOfPlayersJoined > 1 && m_playerIndex == 1 || (m_game->m_numberOfPlayersJoined == 1 && m_playerIndex == 0))
			{
				bounds.m_maxs.y = bounds.m_mins.y + (bounds.m_maxs.y * 0.15f);
			}
			else
			{
				bounds.m_mins.y = bounds.m_maxs.y * 0.5f;
				bounds.m_maxs.y = bounds.m_mins.y + (bounds.m_maxs.y * 0.15f);
			}
			m_hudboxBounds = bounds;
			AddVertsForAABB2D(m_hudBox, bounds, AABB2::ZERO_TO_ONE, Rgba8::WHITE);
		}
	}
	ActorLockedCameraUpdate(deltaSeconds);
	CheckWeaponFire();
	ClampPlayerRotation();
	UpdateSounds();
}
void Player::FreeCameraUpdate(float deltaSeconds)
{
	UNUSED((void)deltaSeconds);
	XBoxController const& controller = g_theInputSystem->GetController(0);
	if (g_theInputSystem->IsKeyDown(16) || g_theInputSystem->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_A))
	{
		m_playerVelocity = 10.0f;
	}
	else
	{
		m_playerVelocity = 1.0f;
	}
	//if (g_theInputSystem->IsKeyDown('H') || g_theInputSystem->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_START))
	//{
	//	m_position = Vec3(0.0f, 0.0f, 0.0f);
	//	m_orientationDegrees = EulerAngles(0.0f, 0.0f, 0.0f);
	//}
	Mat44 playerMatrix = GetModalMatrix();
	if (g_theInputSystem->IsKeyDown('W'))
	{
		m_position += playerMatrix.GetIBasis3D() * deltaSeconds * m_playerVelocity;
	}
	if (g_theInputSystem->IsKeyDown('S'))
	{
		m_position -= playerMatrix.GetIBasis3D() * deltaSeconds * m_playerVelocity;
	}
	if (g_theInputSystem->IsKeyDown('D'))
	{
		m_position -= playerMatrix.GetJBasis3D() * deltaSeconds * m_playerVelocity;
	}
	if (g_theInputSystem->IsKeyDown('A'))
	{
		m_position += playerMatrix.GetJBasis3D() * deltaSeconds * m_playerVelocity;
	}
	if (g_theInputSystem->IsKeyDown('Z'))
	{
		m_position += playerMatrix.GetKBasis3D() * deltaSeconds * m_playerVelocity;
	}
	if (g_theInputSystem->IsKeyDown('C'))
	{
		m_position -= playerMatrix.GetKBasis3D() * deltaSeconds * m_playerVelocity;
	}
	Vec2 mouseDelta = g_theInputSystem->GetMouseClientDelta();
	m_orientationDegrees.m_yawDegrees += (mouseDelta.x * m_mouseSensitivity.x);
	m_orientationDegrees.m_pitchDegrees -= (mouseDelta.y * m_mouseSensitivity.y);

	if (g_theInputSystem->IsKeyDown('Q'))
	{
		m_orientationDegrees.m_rollDegrees -= (m_mouseSensitivity.x * 6.0f);
	}
	if (g_theInputSystem->IsKeyDown('E'))
	{
		m_orientationDegrees.m_rollDegrees += (m_mouseSensitivity.x * 6.0f);
	}


	if (controller.IsConnected())
	{
		float leftStickOrientation;
		float rightStickOrientation;
		Vec2 leftStick;
		Vec2 rightStick;
		if (controller.GetLeftStick().GetMagnitude() > 0.0f)
		{
			leftStickOrientation = controller.GetLeftStick().GetOrientationDegrees();
			leftStick = Vec2::MakeFromPolarDegrees(leftStickOrientation);
		}
		if (controller.GetRightStick().GetMagnitude() > 0.0f)
		{
			rightStickOrientation = controller.GetRightStick().GetOrientationDegrees();
			rightStick = Vec2::MakeFromPolarDegrees(rightStickOrientation);
		}

		if (leftStick.y > 0.2f)
		{
			m_position += playerMatrix.GetIBasis3D() * deltaSeconds * m_playerVelocity;
		}
		if (leftStick.y < -0.2f)
		{
			m_position -= playerMatrix.GetIBasis3D() * deltaSeconds * m_playerVelocity;
		}
		if (leftStick.x > 0.2f)
		{
			m_position -= playerMatrix.GetJBasis3D() * deltaSeconds * m_playerVelocity;
		}
		if (leftStick.x < -0.2f)
		{
			m_position += playerMatrix.GetJBasis3D() * deltaSeconds * m_playerVelocity;
		}

		if (rightStick.y > 0.2f)
		{
			m_orientationDegrees.m_pitchDegrees -= controller.GetRightStick().GetMagnitude() * m_mouseSensitivity.x;
		}
		if (rightStick.y < -0.2f)
		{
			m_orientationDegrees.m_pitchDegrees += controller.GetRightStick().GetMagnitude() * m_mouseSensitivity.x;
		}
		if (rightStick.x > 0.2f)
		{
			m_orientationDegrees.m_yawDegrees -= controller.GetRightStick().GetMagnitude() * m_mouseSensitivity.y;
		}
		if (rightStick.x < -0.2f)
		{
			m_orientationDegrees.m_yawDegrees += controller.GetRightStick().GetMagnitude() * m_mouseSensitivity.y;
		}
		if (g_theInputSystem->GetController(0).GetLeftTrigger() > 0.15f)
		{
			m_orientationDegrees.m_rollDegrees -= (m_mouseSensitivity.x * 10.0f);
		}
		if (g_theInputSystem->GetController(0).GetRightTrigger() > 0.15f)
		{
			m_orientationDegrees.m_rollDegrees += (m_mouseSensitivity.x * 10.0f);
		}
		if (g_theInputSystem->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_RIGHTSHOULDER))
		{
			m_position -= playerMatrix.GetKBasis3D() * deltaSeconds * m_playerVelocity;
		}
		if (g_theInputSystem->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_LEFTSHOULDER))
		{
			m_position += playerMatrix.GetKBasis3D() * deltaSeconds * m_playerVelocity;
		}
	}
}
void Player::ActorLockedCameraUpdate(float deltaSeconds)
{
	if (m_controllerIndex == -1)
	{
		KeyboardInput(deltaSeconds);
	}
	else
	{
		ControllerInput(deltaSeconds);
	}
}
void Player::ControllerInput(float deltaSeconds)
{
	UNUSED((void)deltaSeconds);
	float speed;
	Actor* actor = GetActor();
	XBoxController const& controller = g_theInputSystem->GetController(m_controllerIndex);
	Vec3 movement;
	if (actor == nullptr)
	{
		return;
	}
	Vec3 actorForward = actor->GetForward();
	Vec3 actorJForward = actor->GetJForward();
	if (controller.IsButtonDown(XboxButtonID::XBOX_BUTTON_A))
	{
		speed = actor->m_definition->m_runSpeed;
	}
	else
	{
		speed = actor->m_definition->m_walkSpeed;
	}
	Mat44 playerMatrix = GetModalMatrix();
	if (controller.IsConnected())
	{
		float leftStickOrientation;
		float rightStickOrientation;
		Vec2 leftStick;
		Vec2 rightStick;
		if (controller.GetLeftStick().GetMagnitude() > 0.0f)
		{
			leftStickOrientation = controller.GetLeftStick().GetOrientationDegrees();
			leftStick = Vec2::MakeFromPolarDegrees(leftStickOrientation);
		}
		if (controller.GetRightStick().GetMagnitude() > 0.0f)
		{
			rightStickOrientation = controller.GetRightStick().GetOrientationDegrees();
			rightStick = Vec2::MakeFromPolarDegrees(rightStickOrientation);
		}

		if (leftStick.y > 0.2f)
		{
			movement.x += m_playerVelocity;
		}
		if (leftStick.y < -0.2f)
		{
			movement.x -= m_playerVelocity;
		}
		if (leftStick.x > 0.2f)
		{
			movement.y -= m_playerVelocity;
		}
		if (leftStick.x < -0.2f)
		{
			movement.y += m_playerVelocity;
		}

		if (rightStick.y > 0.2f)
		{
			m_orientationDegrees.m_pitchDegrees -= controller.GetRightStick().GetMagnitude();
		}
		if (rightStick.y < -0.2f)
		{
			m_orientationDegrees.m_pitchDegrees += controller.GetRightStick().GetMagnitude();
		}
		if (rightStick.x > 0.2f)
		{
			actor->m_orientationDegrees.m_yawDegrees -= controller.GetRightStick().GetMagnitude();
			m_orientationDegrees.m_yawDegrees -= controller.GetRightStick().GetMagnitude() ;

		}
		if (rightStick.x < -0.2f)
		{
			actor->m_orientationDegrees.m_yawDegrees += controller.GetRightStick().GetMagnitude();
			m_orientationDegrees.m_yawDegrees += controller.GetRightStick().GetMagnitude();
		}
	}

	Vec3 totalMovement = (actorForward * movement.x + actorJForward * movement.y) * speed * actor->m_definition->m_drag;
	if (totalMovement.GetLength() != 0.0f)
	{
		actor->AddForce(totalMovement);
	}
}
void Player::KeyboardInput(float deltaSeconds)
{
	UNUSED((void)deltaSeconds);
	float speed;
	Actor* actor = GetActor();
	Vec3 movement;
	if (actor == nullptr)
	{
		return;
	}
	Vec3 actorForward = actor->GetForward();
	Vec3 actorJForward = actor->GetJForward();
	if (g_theInputSystem->IsKeyDown(16) || g_theInputSystem->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_A))
	{
		speed = actor->m_definition->m_runSpeed;
	}
	else
	{
		speed = actor->m_definition->m_walkSpeed;
	}
	Mat44 playerMatrix = GetModalMatrix();
	if (g_theInputSystem->IsKeyDown('W'))
	{
		movement.x += m_playerVelocity;
	}
	if (g_theInputSystem->IsKeyDown('S'))
	{
		movement.x -= m_playerVelocity;
	}
	if (g_theInputSystem->IsKeyDown('D'))
	{
		movement.y -= m_playerVelocity;
	}
	if (g_theInputSystem->IsKeyDown('A'))
	{
		movement.y += m_playerVelocity;
	}

	Vec2 mouseDelta = g_theInputSystem->GetMouseClientDelta();
	actor->m_orientationDegrees.m_yawDegrees += (mouseDelta.x * m_mouseSensitivity.x);
	m_orientationDegrees.m_yawDegrees += (mouseDelta.x * m_mouseSensitivity.x);
	m_orientationDegrees.m_pitchDegrees -= (mouseDelta.y * m_mouseSensitivity.y);

	Vec3 totalMovement = (actorForward * movement.x + actorJForward * movement.y) * speed * actor->m_definition->m_drag;
	if (totalMovement.GetLength() != 0.0f)
	{
		actor->AddForce(totalMovement);
	}

}
void Player::ClampPlayerRotation()
{
	m_orientationDegrees.m_pitchDegrees= Clamp(m_orientationDegrees.m_pitchDegrees, -85.0f, 85.0f);
	m_orientationDegrees.m_rollDegrees = Clamp(m_orientationDegrees.m_rollDegrees, -45.0f, 45.0f);
}
void Player::CheckWeaponFire()
{
	if (m_controllerIndex == -1)
	{
		GetMouseInputAndFireWeapon();
	}
	else
	{
		GetControllerInputAndFireWeapon();
	}
}
void Player::GetMouseInputAndFireWeapon()
{
	Actor* actor = GetActor();
	if (actor == nullptr)
	{
		return;
	}
	Mat44 playerMatrix = GetModalMatrix();
	Vec3 startPosition = actor->m_position;
	if (g_theInputSystem->WasKeyJustPressed(LEFT_ARROW))
	{
		actor->EquipPreviousWeapon();
		WeaponSwitch();
	}
	if (g_theInputSystem->WasKeyJustPressed(RIGHT_ARROW))
	{
		actor->EquipNextWeapon();
		WeaponSwitch();
	}
	if (g_theInputSystem->WasKeyJustPressed('1'))
	{
		actor->EquipWeapon(0);
		WeaponSwitch();
	}
	if (g_theInputSystem->WasKeyJustPressed('2'))
	{
		actor->EquipWeapon(1);
		WeaponSwitch();
	}
	if (g_theInputSystem->WasKeyJustPressed('3'))
	{
		actor->EquipWeapon(2);
		WeaponSwitch();
	}
	if (g_theInputSystem->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		playerMatrix = GetModalMatrix();
		startPosition.z = m_position.z;
		if (actor->GetEquippedWeapon())
		{
			actor->GetEquippedWeapon()->Fire(m_position, startPosition, playerMatrix.GetIBasis3D(), actor);
		}
	}
}
void Player::GetControllerInputAndFireWeapon()
{
	XBoxController const& controller = g_theInputSystem->GetController(m_controllerIndex);
	Actor* actor = GetActor();
	if (actor == nullptr)
	{
		return;
	}
	Mat44 playerMatrix = GetModalMatrix();
	Vec3 startPosition = actor->m_position;
	if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_DPADDOWN))
	{
		actor->EquipPreviousWeapon();
		WeaponSwitch();
	}
	if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_DPADUP))
	{
		actor->EquipNextWeapon();
		WeaponSwitch();
	}
	if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_RIGHTSHOULDER))
	{
		playerMatrix = GetModalMatrix();
		startPosition.z = m_position.z- 0.3f;
		if (actor->GetEquippedWeapon())
		{
			actor->GetEquippedWeapon()->Fire(m_position, startPosition, playerMatrix.GetIBasis3D(), actor);
		}
	}
}
void Player::ChangedPossessedPlayer(Actor* actor)
{
	Possess(actor);
	m_position = actor->m_position;
	m_orientationDegrees = actor->m_orientationDegrees;
	actor->m_controller = this;
	m_game->UpdateWorldCameraFov(actor->m_definition->m_cameraFOVDegrees);
}
Mat44 Player::GetModalMatrix() const
{
	Mat44 translationMatrix = Mat44::CreateTranslation3D(m_position);
	Mat44 rotationMatrix = m_orientationDegrees.GetAsMatrix_XFwd_YLeft_ZUp();
	translationMatrix.Append(rotationMatrix);
	return translationMatrix;
}

void Player::UpdateCamera(float deltaSeconds)
{

	UNUSED((void)deltaSeconds);
	if (GetActor() != nullptr)
	{
		m_position = GetActor()->m_position;
		m_position.z = GetActor()->m_definition->m_eyeHeight;
		m_orientationDegrees.m_yawDegrees = GetActor()->m_orientationDegrees.m_yawDegrees;
	}
	m_worldCamera.SetTransform(m_position, m_orientationDegrees);
}
void Player::ClampPlayerWithinBounds()
{
	m_position.x = Clamp(m_position.x,0.0f, (float)m_map->m_dimensions.x);
	m_position.y = Clamp(m_position.y, 0.0f, (float)m_map->m_dimensions.y);
}

void Player::WeaponSwitch()
{
	DestroyFire(); 
	Actor* actor = GetActor();
	Weapon* weapon = actor->GetEquippedWeapon();
	if (weapon->m_definition->m_name != "Firethrower")
	{
		return;
	}
	AABB2 fireBounds;
	//SpriteAnimDefinition* currentAnim;
	AABB2 bounds = m_screenCamera.GetCameraBounds(); 
	if (m_game->m_numberOfPlayersJoined > 1 && m_playerIndex == 1 || (m_game->m_numberOfPlayersJoined == 1 && m_playerIndex == 0))
	{
		if (m_game->m_numberOfPlayersJoined == 1)
		{
			fireBounds.m_mins.y = bounds.m_mins.y + (bounds.m_maxs.y * 0.28f);
			fireBounds.m_maxs.y = bounds.m_mins.y + (bounds.m_maxs.y * 0.50f);
		}
		else
		{
			fireBounds.m_mins.y = bounds.m_mins.y + (bounds.m_maxs.y * 0.20f);
			fireBounds.m_maxs.y = bounds.m_mins.y + (bounds.m_maxs.y * 0.30f);
		}
	}
	else
	{
		fireBounds.m_mins.y = bounds.m_mins.y + (bounds.m_maxs.y * 0.20f) + bounds.m_maxs.y * 0.5f;
		fireBounds.m_maxs.y = bounds.m_mins.y + (bounds.m_maxs.y * 0.30f) + bounds.m_maxs.y * 0.5f;
	}
	fireBounds.m_mins.x = bounds.m_maxs.x * 0.53f;
	fireBounds.m_maxs.x = bounds.m_maxs.x * 0.47f;
	//AddVertsForSphere3D(m_fireBox, 1.f, Vec3(fireBounds.GetCenter()), AABB2::ZERO_TO_ONE, Rgba8::WHITE);
	AddVertsForAABB2D(m_fireBox, fireBounds,AABB2::ZERO_TO_ONE, Rgba8::WHITE);
	//if (m_fireWeponSound == MISSING_SOUND_ID)
	//{
	//	SoundID sound = g_theAudioSystem->CreateOrGetSound("Data/Audio/FireCackle.mp3");
	//	m_fireWeponSound = g_theAudioSystem->StartSoundAt(sound, m_position, 0.6f);
	//}
	//else
	//{
	//	m_fireWeponSound = g_theAudioSystem->StartSoundAt(m_fireWeponSound, m_position, 0.6f);
	//}
}

void Player::DestroyFire()
{
	m_fireBox.clear();
}

void Player::UpdateSounds()
{
	Vec3 i, j, k;
	m_orientationDegrees.GetAsVectors_XFwd_YLeft_ZUp(i,j,k);
	g_theAudioSystem->UpdateListener(m_playerIndex, m_position, i, k);
	g_theAudioSystem->SetSoundPosition(m_fireWeponSound, m_position);
}
