#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/Controller.hpp"
#include "Engine/Renderer/Renderer.hpp"
class Game;

class Player: public Controller
{
public:
	Player();
	~Player();
	Player(Game* game, Vec3 position, EulerAngles orientationDegrees, EulerAngles angularVelocity);
	void				Render() const;
	void				Update(float deltaSeconds);
	void				FreeCameraUpdate(float deltaSeconds);
	void				ActorLockedCameraUpdate(float deltaSeconds);
	void				ControllerInput(float deltaSeconds);
	void				KeyboardInput(float deltaSeconds);
	void				ClampPlayerRotation();
	void				CheckWeaponFire();
	void				GetMouseInputAndFireWeapon();
	void				GetControllerInputAndFireWeapon();
	void				ChangedPossessedPlayer(Actor* actor);
	Mat44				GetModalMatrix() const;
	void				UpdateCamera(float deltaSeconds) override;
	void				ClampPlayerWithinBounds();
	void				WeaponSwitch() override;	
	void				DestroyFire();
	void				UpdateSounds();
	std::vector<Vertex_PCU> m_hudBox;
	std::vector<Vertex_PCU> m_weaponBox;
	std::vector<Vertex_PCU> m_fireBox;
	std::vector<Vertex_PCU> m_raycastHitBox;
	std::vector<Vertex_PCU> m_reticleBox;
	
public:
	//----------------------MULTIPLAYER VARIABLES-----------------------
	SoundPlaybackID		m_fireWeponSound = MISSING_SOUND_ID;
	AABB2				m_playerScreenBounds;
	AABB2				m_hudboxBounds;
	int					m_playerIndex = -1;
	int					m_controllerIndex = -1;
	int					m_kills = 0;
	int					m_deaths;
	Camera				m_screenCamera;
	Camera				m_worldCamera;
	Vec2				m_normalizedViewport = Vec2(1.0f,1.0f);

	EulerAngles			m_orientationDegrees;
	EulerAngles			m_angularVelocity;
	Vec3				m_position;
	Game*				m_game;
	Camera*				m_camera = nullptr;
	bool				m_freeFlyCameraMode = false;
	float				m_playerVelocity = 1.0f;
	Vec2				m_mouseSensitivity = Vec2(0.1f, 0.1f);
	ID3D11SamplerState* m_samplerState = nullptr;
	ID3D11SamplerState* m_samplerState2 = nullptr;
};

