#include "Game.hpp"
#include "stdlib.h"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <Engine/Math/VertexUtils.hpp>
#include "Engine/Input/InputSystem.hpp"
#include <vector>
#include <Engine/Renderer/SimpleTriangleFont.hpp>
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/TileSetDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/TileMaterialDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/ActorDefinition.hpp"

extern App* g_theApp;
extern Renderer* g_theRenderer;
extern InputSystem* g_theInputSystem;
extern AudioSystem* g_theAudioSystem; 
extern Window* g_theWindow;
RandomNumberGenerator rng;

Game::Game()
{
}
void Game::Startup()
{
	InitializeCameras();
	LoadGameSounds();
	SetupInitialVariables();
	TileMaterialDefinition::InitializeDefinitions();
	TileDefinition::InitializeDefinitions();
	TileSetDefinition::InitializeDefinitions();
	ActorDefinition::InitializeDefinitions("Data/Definitions/ProjectileActorDefinitions.xml");
	WeaponDefinition::InitializeDefinitions("Data/Definitions/WeaponDefinitions.xml");
	ActorDefinition::InitializeDefinitions("Data/Definitions/ActorDefinitions.xml");
	MapDefinition::InitializeDefinitions();
	m_bgMusicPlayback = g_theAudioSystem->StartSound(m_lobbyMusic, true, m_defaultGameSoundVolume);
}
void Game::SetupInitialVariables()
{
	m_IsAttractMode = true;
	m_currentGameState = GameStates::Attract;
	g_theRenderer->SetBlendMode(BlendMode::Opaque);
	m_worldCamera.SetOrthoView();
	g_theInputSystem->SetMouseMode(false, false, false);
}
void Game::Update(float deltaSeconds)
{
	SetWindowSettings();
	UpdateGameState();
	switch (m_currentGameState)
	{
	case GameStates::Attract:			 UpdateAttractMode(deltaSeconds); m_cursorPosition = g_theInputSystem->GetMouseClientPosition();		break;
	case GameStates::Lobby:				 UpdateLobby(deltaSeconds); m_cursorPosition = g_theInputSystem->GetMouseClientPosition();				break;
	case GameStates::GameWorld:			 UpdateGame(deltaSeconds);																				break;
	default:																																	break;
	}
}
void Game::Render() const
{
	Rgba8 clearScreen = Rgba8(0, 0, 0, 255);
	g_theRenderer->ClearScreen(clearScreen);
	switch (m_currentGameState)
	{
	//case GameStates::EngineLogo:		 RenderEngineLogo();							break;
	case GameStates::Attract:			 RenderAttractMode();							break;
	case GameStates::Lobby:				 RenderLobby();									break;
	case GameStates::GameWorld:			 RenderGame();									break;
	default:																			break;
	}
} 
void Game::EndFrame()
{
	if (m_currentGameState == GameStates::GameWorld)
	{
		for (int i = 0; i < (int)m_players.size(); i++)
		{
			m_players[i]->UpdateCamera(0.0f);
			m_players[i]->m_weaponBox.clear();
		}
		m_currentMap->AllActorDeletion();
	}
	m_deadBox.clear();

}

//-----------------------------ATTRACT SCREEN FUNCTIONS------------------------
void Game::RenderAttractMode() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->BindShader(g_theRenderer->GetDefaultShader());
	RenderMenuScreenPlayButton();
	std::vector<Vertex_PCU> gameNameText;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Images/SquirrelFixedFont");
	font->AddVertsForText2D(gameNameText, Vec2(600, 600), 75.0f, "Doomenstein", Rgba8::CYAN, 0.6f, 50.0f);
	font->AddVertsForTextInBox2D(gameNameText, m_screenCamera.GetCameraBounds(), 15.0f, "HIT SPACE TO PLAY", Rgba8(255, 0, 0, 255),
		0.6f, Vec2(0.5f, 0.5f), TextBoxMode::SHRINK_TO_FIT, 999999, 5.0f);
	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(int(gameNameText.size()), gameNameText.data());
	gameNameText.clear();
	font = nullptr;
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->EndCamera(m_screenCamera);
}
void Game::UpdateAttractMode(float deltaSeconds)
{
	UNUSED((void)deltaSeconds);
	//---------------------------CHECKING FOR PLAYER INPUT AND SWITCHING TO GAME SCREEN-----------------------------------------------
	if (g_theInputSystem->WasKeyJustPressed(' ') )
	{
		SwitchToLobby(InputMode::Keyboard,-1);
	}
	if (g_theInputSystem->GetController(0).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))
	{
		SwitchToLobby(InputMode::Controller, 0);
	}
	if (g_theInputSystem->GetController(1).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))
	{
		SwitchToLobby(InputMode::Controller, 1);
	}
	if (g_theInputSystem->GetController(2).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))
	{
		SwitchToLobby(InputMode::Controller, 2);
	}
	if (g_theInputSystem->GetController(3).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))
	{
		SwitchToLobby(InputMode::Controller, 3);
	}

}

//-------------------------------LOBBY FUNCTIONS---------------------------
void Game::SwitchToLobby(InputMode mode, int controllerID)
{
	g_theInputSystem->SetCursorAtCenter();
	m_IsAttractMode = false;
	m_currentGameState = GameStates::Lobby;

	if (mode == InputMode::Controller)
	{
		m_numberOfPlayersJoined++;
		Player* newPlayer = new Player();
		newPlayer->m_controllerIndex = controllerID;
		newPlayer->m_playerIndex = m_numberOfPlayersJoined - 1;
		SetCamerasForPlayer(newPlayer);
		m_players.push_back(newPlayer);
	}
	else if (mode == InputMode::Keyboard)
	{
		m_numberOfPlayersJoined++;
		Player* newPlayer = new Player();
		newPlayer->m_controllerIndex = -1;
		newPlayer->m_playerIndex = m_numberOfPlayersJoined - 1;
		SetCamerasForPlayer(newPlayer);
		m_players.push_back(newPlayer);
	}
	
}
void Game::RenderLobby() const
{
	std::vector<Vertex_PCU> gameNameText;
	g_theRenderer->BindShader(g_theRenderer->GetDefaultShader());
	g_theRenderer->BeginCamera(m_screenCamera);
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Images/SquirrelFixedFont");

	Vec2 alignment = Vec2(0.50f, 0.70f);
	for (int i = 0; i < (int)m_players.size(); i++)
	{
		if (m_players[i] == nullptr) { continue; };
		std::string playerText = Stringf("Player %i ", i+1);
		std::string controlsText;

		if (m_players[i]->m_controllerIndex == -1)
		{
			playerText.append("Mouse and Keyboard");
			if ((int)m_players.size() < m_maxPlayers)
			{
				controlsText = Stringf("Press SPACE to start game \n Press ESCAPE to leave game \n Press Start to join player 2");
			}
			else
			{
				controlsText = Stringf("Press SPACE to start game \n Press ESCAPE to leave game");
			}
		}
		else
		{
			playerText.append("Controller");
			controlsText = Stringf("Press START to start game \n Press BACK to leave game");
		}

		font->AddVertsForTextInBox2D(gameNameText, m_screenCamera.GetCameraBounds(), 30.0f, playerText, Rgba8::WHITE,
			0.6f, alignment, TextBoxMode::SHRINK_TO_FIT, 999999, 5.0f);

		alignment.y -= 0.10f;
		font->AddVertsForTextInBox2D(gameNameText, m_screenCamera.GetCameraBounds(), 20.0f, controlsText, Rgba8::WHITE,
			0.6f, alignment, TextBoxMode::SHRINK_TO_FIT, 999999, 5.0f);

		alignment.y -= 0.20f;
	}
	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(int(gameNameText.size()), gameNameText.data());
	gameNameText.clear();
	font = nullptr;
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->EndCamera(m_screenCamera);
}
void Game::UpdateLobby(float deltaSeconds)
{
	UNUSED((void)deltaSeconds);
	CheckPlayersBackingOut();
	CheckPlayersSwitchingToGame();

	Player* newPlayer = nullptr;
	if (g_theInputSystem->WasKeyJustPressed(' ') && m_players.size() < m_maxPlayers && !CheckIfControllerJoined(-1))
	{
		m_numberOfPlayersJoined++;
		newPlayer = new Player();
		newPlayer->m_playerIndex = m_numberOfPlayersJoined - 1;
		m_players.push_back(newPlayer);
		g_theInputSystem->ConsumeKeyJustPressed(' ');
	}
	if (g_theInputSystem->GetController(0).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START) && m_players.size() < m_maxPlayers && !CheckIfControllerJoined(0))
	{
		m_numberOfPlayersJoined++;
		newPlayer = new Player();
		newPlayer->m_playerIndex = m_numberOfPlayersJoined - 1;
		newPlayer->m_controllerIndex = 0;
		m_players.push_back(newPlayer);
	}
	if (g_theInputSystem->GetController(1).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START) && m_players.size() < m_maxPlayers && !CheckIfControllerJoined(1))
	{
		m_numberOfPlayersJoined++;
		newPlayer = new Player();
		newPlayer->m_playerIndex = m_numberOfPlayersJoined - 1;
		newPlayer->m_controllerIndex = 1;
		m_players.push_back(newPlayer);
	}
	if (g_theInputSystem->GetController(2).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START) && m_players.size() < m_maxPlayers && !CheckIfControllerJoined(2))
	{
		m_numberOfPlayersJoined++;
		newPlayer = new Player();
		newPlayer->m_playerIndex = m_numberOfPlayersJoined - 1;
		newPlayer->m_controllerIndex = 2;
		m_players.push_back(newPlayer);
	}
	if (g_theInputSystem->GetController(3).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START) && m_players.size() < m_maxPlayers && !CheckIfControllerJoined(3))
	{
		m_numberOfPlayersJoined++;
		newPlayer = new Player();
		newPlayer->m_playerIndex = m_numberOfPlayersJoined - 1;
		newPlayer->m_controllerIndex = 3;
		m_players.push_back(newPlayer);
	}
	//------------------------SWITCH TO ATTRACT-----------------------
	if (m_numberOfPlayersJoined == 0)
	{
		SwitchGameModes(GameStates::Attract);
	}
}
void Game::SetCamerasForPlayer(Player* player)
{
	player->m_game = this;
	Vec3 ibasis = Vec3(0.0f, 0.0f, 1.0f);
	Vec3 jbasis = Vec3(-1.0f, 0.0f, 0.0f);
	Vec3 kbasis = Vec3(0.0f, 1.0f, 0.0f);
	if (m_numberOfPlayersJoined == 2)
	{
		player->m_worldCamera.m_cameraAspectRatio = g_theWindow->GetConfig().m_clientAspect * 2;
		player->m_worldCamera.SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect * 2, 60.0f, 0.1f, 100.0f);
		player->m_worldCamera.SetViewToRenderTransform(ibasis, jbasis, kbasis);
		if (player->m_playerIndex == 0)
		{
			player->m_worldCamera.m_viewport = AABB2(Vec2(0.0f, 0.0f), Vec2(1.0f,0.5f));
		}
		else if (player->m_playerIndex == 1)
		{
			player->m_worldCamera.m_viewport = AABB2(Vec2(0.0f, 0.5f), Vec2(1.0f, 0.5f));
		}
	}
	else
	{
		player->m_worldCamera = m_worldCamera;
		player->m_worldCamera.m_cameraAspectRatio = 2.0f;
		player->m_worldCamera.SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.0f, 0.01f, 100.0f);
		player->m_worldCamera.SetViewToRenderTransform(ibasis, jbasis, kbasis);
	}
	player->m_screenCamera = m_screenCamera;
}
void Game::CheckPlayersBackingOut()
{
	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		for (int i = 0; i < (int)m_players.size(); i++)
		{
			if (m_players[i])
			{
				if (m_players[i]->m_controllerIndex == -1)
				{
					delete m_players[i];
					m_players[i] = nullptr;
					m_numberOfPlayersJoined -= 1;
				}
			}
		}
	}
	if (g_theInputSystem->GetController(0).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_BACK))
	{
		for (int i = 0; i < (int)m_players.size(); i++)
		{
			if (m_players[i])
			{
				if (m_players[i]->m_controllerIndex == 0)
				{
					delete m_players[i];
					m_players[i] = nullptr;
					m_numberOfPlayersJoined -= 1;
				}
			}
		}
	}
	if (g_theInputSystem->GetController(1).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_BACK))
	{
		for (int i = 0; i < (int)m_players.size(); i++)
		{
			if (m_players[i])
			{
				if (m_players[i]->m_controllerIndex == 1)
				{
					delete m_players[i];
					m_players[i] = nullptr;
					m_numberOfPlayersJoined -= 1;
				}
			}
		}
	}
	if (g_theInputSystem->GetController(2).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_BACK))
	{
		for (int i = 0; i < (int)m_players.size(); i++)
		{
			if (m_players[i])
			{
				if (m_players[i]->m_controllerIndex == 2)
				{
					delete m_players[i];
					m_players[i] = nullptr;
					m_numberOfPlayersJoined -= 1;
				}
			}
		}
	}
	if (g_theInputSystem->GetController(3).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_BACK))
	{
		for (int i = 0; i < (int)m_players.size(); i++)
		{
			if (m_players[i])
			{
				if (m_players[i]->m_controllerIndex == 3)
				{
					delete m_players[i];
					m_players[i] = nullptr;
					m_numberOfPlayersJoined -= 1;
				}
			}
		}
	}

}
void Game::CheckPlayersSwitchingToGame()
{
	if (g_theInputSystem->WasKeyJustPressed(' '))
	{
		for (int i = 0; i < (int)m_players.size(); i++)
		{
			if (m_players[i])
			{
				if (m_players[i]->m_controllerIndex == -1)
				{
					SwitchToGame(0.0f);
				}
			}
		}
	}
	if (g_theInputSystem->GetController(0).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))
	{
		for (int i = 0; i < (int)m_players.size(); i++)
		{
			if (m_players[i])
			{
				if (m_players[i]->m_controllerIndex == 0)
				{
					SwitchToGame(0.0f);
				}
			}
		}
	}
	if (g_theInputSystem->GetController(1).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))
	{
		for (int i = 0; i < (int)m_players.size(); i++)
		{
			if (m_players[i])
			{
				if (m_players[i]->m_controllerIndex == 1)
				{
					SwitchToGame(0.0f);
				}
			}
		}
	}
	if (g_theInputSystem->GetController(2).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))
	{
		for (int i = 0; i < (int)m_players.size(); i++)
		{
			if (m_players[i])
			{
				if (m_players[i]->m_controllerIndex == 2)
				{
					SwitchToGame(0.0f);
				}
			}
		}
	}
	if (g_theInputSystem->GetController(3).WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))
	{
		for (int i = 0; i < (int)m_players.size(); i++)
		{
			if (m_players[i])
			{
				if (m_players[i]->m_controllerIndex == 3)
				{
					SwitchToGame(0.0f);
				}
			}
		}
	}

}
bool Game::CheckIfControllerJoined(int controllerIndex)
{
	for (int i = 0; i < (int)m_players.size(); i++)
	{
		if (m_players[i])
		{
			if (m_players[i]->m_controllerIndex == controllerIndex)
			{
				return true;
			}
		}
	}
	return false;
}
//-----------------------------GAME SCREEN FUNCTIONS--------------------------
void Game::SwitchToGame(float deltaSeconds)
{
	for (int i = 0; i < (int)m_players.size(); i++)
	{
		SetCamerasForPlayer(m_players[i]);
	}
	g_theAudioSystem->SetNumListeners(m_numberOfPlayersJoined);
	UNUSED((void)deltaSeconds);
	g_theInputSystem->SetCursorAtCenter();
	m_IsAttractMode = false;
	m_currentGameState = GameStates::GameWorld;
	CreateMap();
	m_worldCamera.SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.0f, 0.1f, 100.0f);
	g_theAudioSystem->StopSound(m_bgMusicPlayback);
	m_bgMusicPlayback = g_theAudioSystem->StartSound(m_gameMusic, true, m_defaultGameSoundVolume);
}
void Game::RenderGame() const
{
	for (int i = 0; i < (int)m_players.size(); i++)
	{
		g_theRenderer->BeginCamera(m_players[i]->m_worldCamera);
		if (m_currentMap)
		{
			m_currentMap->m_currentPlayerRendering++;
			m_currentMap->Render();
		}
	}
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->EndCamera(m_worldCamera);

	for (int i = 0; i < (int)m_players.size(); i++)
	{
		m_players[i]->Render();
	}


}
void Game::UpdateGame(float deltaSeconds)
{
	g_theRenderer->m_frameTime += deltaSeconds;
	if (g_theRenderer->m_frameTime > 100.0f)
	{
		g_theRenderer->m_frameTime = 0.0f;
	}
	CheckPlayerDeadAndRespawn();
	DrawDebugScreenTexts(deltaSeconds);
	m_currentMap->Update(deltaSeconds);
	m_currentMap->m_currentPlayerRendering = -1;
}

void Game::SetWindowSettings()
{
	bool windowHasFocus = g_theRenderer->GetRenderConfig().m_window->HasFocus();
	if (!windowHasFocus)
	{
		g_theInputSystem->SetMouseMode(false, false, false);
	}
	else if (windowHasFocus)
	{
		if (g_theConsole->GetMode() == DevConsoleMode::VISIBLE)
		{
			g_theInputSystem->SetMouseMode(false, false, false);
		}
		else
		{
			if (m_IsAttractMode)
			{
				g_theInputSystem->SetMouseMode(false, false, false);
			}
			else
			{
				g_theInputSystem->SetMouseMode(true, true, true);
			}
		}
	}
}
void Game::CheckPlayerDeadAndRespawn()
{
	for (int i = 0; i < (int)m_players.size(); i++)
	{
		if (m_players[i]->GetActor() != nullptr)
		{
			if (m_players[i]->GetActor()->m_isDestroyed)
			{
				m_players[i]->m_deaths++;
				m_currentMap->SpawnNewActorForPlayer(m_players[i]);
			}
		}
		else if (m_players[i]->GetActor() == nullptr)
		{
			m_players[i]->m_deaths++;
			m_currentMap->SpawnNewActorForPlayer(m_players[i]);
		}
		
	}
}
void Game::UpdateWorldCameraFov(float fov)
{
	m_worldCamera.SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, fov, 0.1f, 100.0f);
}
void Game::DrawDebugScreenTexts(float deltaSeconds)
{
	UNUSED((void)deltaSeconds);
	float scaleGameClock = (float)g_theApp->m_gameClock.GetTimeDilation();
	//std::string playerPositionText = Stringf("Player Position: %.2f, %.2f,%.2f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z);
	//DebugAddScreenText(playerPositionText, Vec2(), 2.0f, Vec2(0.0f, 1.0f), 1.0f, Rgba8::WHITE, Rgba8::RED);
	//std::string devConsoleClockText = Stringf("Dev Console    | Time: %.2f  Fps: %.2f  Scale: %.2f", g_theConsole->m_clock.GetTotalTime(), 1.0f / g_theConsole->m_clock.GetDeltaTime(), 1.0f);
	//DebugAddScreenText(devConsoleClockText, Vec2(), -1.0f, Vec2(1.0f, 1.0f), 1.0f, Rgba8::WHITE, Rgba8::RED);
	//std::string debugClockText = Stringf("Debug Render    | Time: %.2f  Fps: %.2f  Scale: %.2f", DebugRenderGetClock()->GetTotalTime(), 1.0f / DebugRenderGetClock()->GetDeltaTime(), scaleDebugClock);
	//DebugAddScreenText(debugClockText, Vec2(), -1.0f, Vec2(1.0f, 0.98f), 1.0f, Rgba8::WHITE, Rgba8::RED);
	std::string gameClockText = Stringf("Game    | Time: %.2f  Fps: %.2f  Scale: %.2f", g_theApp->m_gameClock.GetTotalTime(), 1.0f / g_theApp->m_gameClock.GetDeltaTime(), scaleGameClock);
	DebugAddScreenText(gameClockText, Vec2(), -1.0f, Vec2(1.0f, 0.96f), 1.0f, Rgba8::WHITE, Rgba8::RED);
}
void Game::PauseGame(bool pauseGame)
{
	UNUSED((void)pauseGame);
	//m_isGamePaused = pauseGame;
	//m_masterVolume = 0.0f;
	//if (!(m_currentGameState == GameStates::Attract) && !(m_currentGameState == GameStates::EngineLogo))
	//{
	//	if (m_isGamePaused)
	//	{
	//	}
	//	else
	//	{
	//		m_masterVolume = 1.0f;
	//	}
	//}
}
Camera Game::GetWorldCamera()
{
	return m_worldCamera;
}
void Game::DebugRender() const
{

}
void Game::ShutDown()
{
}
void Game::GetMouseInputAndSpawnRaycasts()
{
	//Mat44 playerMatrix = m_player->GetModalMatrix();
	//Vec3 startPosition = m_player->m_position;
	//if (g_theInputSystem->WasKeyJustPressed(KEYCODE_LEFT_MOUSE) && g_theConsole->m_raycastMode == RaycastToggleMode::RaycastVisible)
	//{
	//	Vec3 endPosition = startPosition +  playerMatrix.GetIBasis3D() * 10.0f;
	//	if (m_currentMap)
	//	{
	//		Vec3 direction = playerMatrix.GetIBasis3D().GetNormalized();
	//		RaycastResult result = m_currentMap->RaycastAll(startPosition, direction, 10.0f);
	//		if (result.m_didImpact)
	//		{
	//			DebugAddWorldArrow(result.m_impactPosition, result.m_impactSurfaceNormal, 0.03f, 10.0f, Rgba8::BLUE, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::USEDEPTH);
	//			DebugAddWorldSphere(result.m_impactPosition, 0.06f, 10.0f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::USEDEPTH);
	//		}
	//	}
	//	/*DebugAddWorldArrow(Vec3(8.0f,12.6f,0.32f),Vec3(7.69f,12.6f,0.32f), 0.03f, 10.0f, Rgba8::BLUE, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::USEDEPTH);*/
	//	DebugAddWorldLine(m_player->m_position, endPosition, 0.01f, 10.0f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::XRAY);
	//}
	//if (g_theInputSystem->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE) && g_theConsole->m_raycastMode == RaycastToggleMode::RaycastVisible)
	//{
	//	Vec3 endPosition = startPosition + playerMatrix.GetIBasis3D() * 0.25f;
	//	if (m_currentMap)
	//	{
	//		Vec3 direction = playerMatrix.GetIBasis3D().GetNormalized();
	//		RaycastResult result = m_currentMap->RaycastAll(startPosition, direction, 0.25f);
	//		if (result.m_didImpact)
	//		{
	//			DebugAddWorldArrow(result.m_impactPosition, result.m_impactSurfaceNormal, 0.03f, 10.0f, Rgba8::BLUE, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::USEDEPTH);
	//			DebugAddWorldSphere(result.m_impactPosition, 0.06f, 10.0f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::USEDEPTH);
	//		}
	//	}
	//	DebugAddWorldLine(m_player->m_position, endPosition, 0.01f, 10.0f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::XRAY);
	//}
}
//void Game::GetInputAndMovePlayerFreeFlow(float deltaSeconds)
//{
//	XBoxController const& controller = g_theInputSystem->GetController(0);
//	if (g_theInputSystem->IsKeyDown(16) || g_theInputSystem->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_A))
//	{
//		m_playerVelocity = 10.0f;
//	}
//	else
//	{
//		m_playerVelocity = 1.0f;
//	}
//	if (g_theInputSystem->IsKeyDown('H') || g_theInputSystem->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_START))
//	{
//		m_player->m_position = Vec3(0.0f, 0.0f, 0.0f);
//		m_player->m_orientationDegrees = EulerAngles(0.0f, 0.0f, 0.0f);
//	}
//	Mat44 playerMatrix = m_player->GetModalMatrix();
//	if (g_theInputSystem->IsKeyDown('W'))
//	{
//		m_player->m_position += playerMatrix.GetIBasis3D() * deltaSeconds * m_playerVelocity;
//	}
//	if (g_theInputSystem->IsKeyDown('S'))
//	{
//		m_player->m_position -= playerMatrix.GetIBasis3D() * deltaSeconds * m_playerVelocity;
//	}
//	if (g_theInputSystem->IsKeyDown('D'))
//	{
//		m_player->m_position -= playerMatrix.GetJBasis3D() * deltaSeconds * m_playerVelocity;
//	}
//	if (g_theInputSystem->IsKeyDown('A'))
//	{
//		m_player->m_position += playerMatrix.GetJBasis3D() * deltaSeconds * m_playerVelocity;
//	}
//	if (g_theInputSystem->IsKeyDown('Z'))
//	{
//		m_player->m_position += playerMatrix.GetKBasis3D() * deltaSeconds * m_playerVelocity;
//	}
//	if (g_theInputSystem->IsKeyDown('C'))
//	{
//		m_player->m_position -= playerMatrix.GetKBasis3D() * deltaSeconds * m_playerVelocity;
//	}
//	Vec2 mouseDelta = g_theInputSystem->GetMouseClientDelta();
//	m_player->m_orientationDegrees.m_yawDegrees += (mouseDelta.x * m_mouseSensitivity.x);
//	m_player->m_orientationDegrees.m_pitchDegrees -= (mouseDelta.y * m_mouseSensitivity.y);
//
//	if (g_theInputSystem->IsKeyDown('Q'))
//	{
//		m_player->m_orientationDegrees.m_rollDegrees -= (m_mouseSensitivity.x * 6.0f);
//	}
//	if (g_theInputSystem->IsKeyDown('E'))
//	{
//		m_player->m_orientationDegrees.m_rollDegrees += (m_mouseSensitivity.x * 6.0f);
//	}
//
//	
//	if (controller.IsConnected())
//	{
//		float leftStickOrientation;
//		float rightStickOrientation;
//		Vec2 leftStick;
//		Vec2 rightStick;
//		if (controller.GetLeftStick().GetMagnitude() > 0.0f)
//		{
//			leftStickOrientation = controller.GetLeftStick().GetOrientationDegrees();
//			leftStick = Vec2::MakeFromPolarDegrees(leftStickOrientation);
//		}
//		if (controller.GetRightStick().GetMagnitude() > 0.0f)
//		{
//			rightStickOrientation = controller.GetRightStick().GetOrientationDegrees();
//			rightStick = Vec2::MakeFromPolarDegrees(rightStickOrientation);
//		}
//
//		if (leftStick.y > 0.2f)
//		{
//			m_player->m_position += playerMatrix.GetIBasis3D() * deltaSeconds * m_playerVelocity;
//		}
//		if (leftStick.y <-0.2f)
//		{
//			m_player->m_position -= playerMatrix.GetIBasis3D() * deltaSeconds * m_playerVelocity;
//		}
//		if (leftStick.x > 0.2f)
//		{
//			m_player->m_position -= playerMatrix.GetJBasis3D() * deltaSeconds * m_playerVelocity;
//		}
//		if (leftStick.x < -0.2f)
//		{
//			m_player->m_position += playerMatrix.GetJBasis3D() * deltaSeconds * m_playerVelocity;
//		}
//
//		if (rightStick.y > 0.2f)
//		{
//			m_player->m_orientationDegrees.m_pitchDegrees -= controller.GetRightStick().GetMagnitude() * m_mouseSensitivity.x;
//		}
//		if (rightStick.y < -0.2f)
//		{
//			m_player->m_orientationDegrees.m_pitchDegrees += controller.GetRightStick().GetMagnitude() * m_mouseSensitivity.x;
//		}
//		if (rightStick.x > 0.2f)
//		{
//			m_player->m_orientationDegrees.m_yawDegrees -= controller.GetRightStick().GetMagnitude() * m_mouseSensitivity.y;
//		}
//		if (rightStick.x < -0.2f)
//		{
//			m_player->m_orientationDegrees.m_yawDegrees += controller.GetRightStick().GetMagnitude() * m_mouseSensitivity.y;
//		}
//		if (g_theInputSystem->GetController(0).GetLeftTrigger() > 0.15f)
//		{
//			m_player->m_orientationDegrees.m_rollDegrees -= (m_mouseSensitivity.x * 10.0f);
//		}
//		if (g_theInputSystem->GetController(0).GetRightTrigger() > 0.15f)
//		{
//			m_player->m_orientationDegrees.m_rollDegrees += (m_mouseSensitivity.x * 10.0f);
//		}
//		if (g_theInputSystem->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_RIGHTSHOULDER))
//		{
//			m_player->m_position -= playerMatrix.GetKBasis3D() * deltaSeconds * m_playerVelocity;
//		}
//		if (g_theInputSystem->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_LEFTSHOULDER))
//		{
//			m_player->m_position += playerMatrix.GetKBasis3D() * deltaSeconds * m_playerVelocity;
//		}
//	}
//	m_player->Update(deltaSeconds);
//}
//void Game::GetInputAndMovePlayer(float deltaSeconds)
//{
//	XBoxController const& controller = g_theInputSystem->GetController(0);
//	if (g_theInputSystem->IsKeyDown(16) || g_theInputSystem->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_A))
//	{
//		m_playerVelocity = 10.0f;
//	}
//	else
//	{
//		m_playerVelocity = 1.0f;
//	}
//	if (g_theInputSystem->IsKeyDown('H') || g_theInputSystem->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_START))
//	{
//		m_player->m_position = Vec3(0.0f, 0.0f, 0.0f);
//		m_player->m_orientationDegrees = EulerAngles(0.0f, 0.0f, 0.0f);
//	}
//	Mat44 playerMatrix = m_player->GetModalMatrix();
//	if (g_theInputSystem->IsKeyDown('W'))
//	{
//		m_player->m_position.x +=  deltaSeconds * m_playerVelocity;
//	}
//	if (g_theInputSystem->IsKeyDown('S'))
//	{
//		m_player->m_position.x -=  deltaSeconds * m_playerVelocity;
//	}
//	if (g_theInputSystem->IsKeyDown('D'))
//	{
//		m_player->m_position.y -= deltaSeconds * m_playerVelocity;
//	}
//	if (g_theInputSystem->IsKeyDown('A'))
//	{
//		m_player->m_position.y += deltaSeconds * m_playerVelocity;
//	}
//
//	Vec2 mouseDelta = g_theInputSystem->GetMouseClientDelta();
//	m_player->m_orientationDegrees.m_yawDegrees += (mouseDelta.x * m_mouseSensitivity.x);
//	m_player->m_orientationDegrees.m_pitchDegrees -= (mouseDelta.y * m_mouseSensitivity.y);
//
//	if (g_theInputSystem->IsKeyDown('Q'))
//	{
//		m_player->m_orientationDegrees.m_rollDegrees -= (m_mouseSensitivity.x * 6.0f);
//	}
//	if (g_theInputSystem->IsKeyDown('E'))
//	{
//		m_player->m_orientationDegrees.m_rollDegrees += (m_mouseSensitivity.x * 6.0f);
//	}
//
//
//	if (controller.IsConnected())
//	{
//		float leftStickOrientation;
//		float rightStickOrientation;
//		Vec2 leftStick;
//		Vec2 rightStick;
//		if (controller.GetLeftStick().GetMagnitude() > 0.0f)
//		{
//			leftStickOrientation = controller.GetLeftStick().GetOrientationDegrees();
//			leftStick = Vec2::MakeFromPolarDegrees(leftStickOrientation);
//		}
//		if (controller.GetRightStick().GetMagnitude() > 0.0f)
//		{
//			rightStickOrientation = controller.GetRightStick().GetOrientationDegrees();
//			rightStick = Vec2::MakeFromPolarDegrees(rightStickOrientation);
//		}
//
//		if (leftStick.y > 0.2f)
//		{
//			m_player->m_position.x +=  deltaSeconds * m_playerVelocity;
//		}
//		if (leftStick.y < -0.2f)
//		{
//			m_player->m_position.x -= deltaSeconds * m_playerVelocity;
//		}
//		if (leftStick.x > 0.2f)
//		{
//			m_player->m_position.y -=  deltaSeconds * m_playerVelocity;
//		}
//		if (leftStick.x < -0.2f)
//		{
//			m_player->m_position.y +=  deltaSeconds * m_playerVelocity;
//		}
//
//		if (rightStick.y > 0.2f)
//		{
//			m_player->m_orientationDegrees.m_pitchDegrees -= controller.GetRightStick().GetMagnitude() * m_mouseSensitivity.x;
//		}
//		if (rightStick.y < -0.2f)
//		{
//			m_player->m_orientationDegrees.m_pitchDegrees += controller.GetRightStick().GetMagnitude() * m_mouseSensitivity.x;
//		}
//		if (rightStick.x > 0.2f)
//		{
//			m_player->m_orientationDegrees.m_yawDegrees -= controller.GetRightStick().GetMagnitude() * m_mouseSensitivity.y;
//		}
//		if (rightStick.x < -0.2f)
//		{
//			m_player->m_orientationDegrees.m_yawDegrees += controller.GetRightStick().GetMagnitude() * m_mouseSensitivity.y;
//		}
//		if (g_theInputSystem->GetController(0).GetLeftTrigger() > 0.15f)
//		{
//			m_player->m_orientationDegrees.m_rollDegrees -= (m_mouseSensitivity.x * 10.0f);
//		}
//		if (g_theInputSystem->GetController(0).GetRightTrigger() > 0.15f)
//		{
//			m_player->m_orientationDegrees.m_rollDegrees += (m_mouseSensitivity.x * 10.0f);
//		}
//		if (g_theInputSystem->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_RIGHTSHOULDER))
//		{
//			m_player->m_position -= playerMatrix.GetKBasis3D() * deltaSeconds * m_playerVelocity;
//		}
//		if (g_theInputSystem->GetController(0).IsButtonDown(XboxButtonID::XBOX_BUTTON_LEFTSHOULDER))
//		{
//			m_player->m_position += playerMatrix.GetKBasis3D() * deltaSeconds * m_playerVelocity;
//		}
//	}
//	ClampPlayerWithinMapBounds();
//	for (int i = 0; i < (int)m_players.size(); i++)
//	{
//		m_players[i]->Update(deltaSeconds);
//	}
//}
void Game::CreateMap()
{
	std::string defaultValue			 = "";
	std::string mapName					 = g_gameConfigBlackboard.GetValue("MAP_NAME", defaultValue);
	const MapDefinition* mapDefinition	 = MapDefinition::GetByName(mapName);
	m_currentMap						 = new Map(this, mapDefinition);
	if (m_currentMap)
	{
		m_currentMap->CreateTiles();
		m_currentMap->CreateInitialActors();
	}
}
void Game::DeleteMap()
{
	if (m_currentMap)
	{
		delete m_currentMap;
		m_currentMap = nullptr;
	}
}
void Game::ClampPlayerWithinMapBounds()
{
	/*m_player->m_position.x = Clamp(m_player->m_position.x,0.0f, (float)m_currentMap->m_dimensions.x);
	m_player->m_position.y = Clamp(m_player->m_position.y, 0.0f, (float)m_currentMap->m_dimensions.y);*/
}


void Game::InitializeCameras()
{
	m_worldCamera = Camera(-1.0f, -1.0f, CAMERA_WORLD_SIZEX, CAMERA_WORLD_SIZEY);
	m_screenCamera = Camera(0.0f, 0.0f, CAMERA_SCREEN_SIZEX, CAMERA_SCREEN_SIZEY);

	Vec3 ibasis = Vec3(0.0f, 0.0f, 1.0f);
	Vec3 jbasis = Vec3(-1.0f, 0.0f, 0.0f);
	Vec3 kbasis = Vec3(0.0f, 1.0f, 0.0f);
	m_worldCamera.SetViewToRenderTransform(ibasis,jbasis,kbasis);
}
void Game::LoadGameSounds()
{
	m_gameSounds[(int)GameSounds::GamePause] = g_theAudioSystem->CreateOrGetSound("Data/Audio/Pause.mp3");
	m_gameSounds[(int)GameSounds::GameVictory] = g_theAudioSystem->CreateOrGetSound("Data/Audio/Victory.mp3");
	m_lobbyMusic = g_theAudioSystem->CreateOrGetSound("Data/Audio/Music/MainMenu_InTheDark.mp2");
	m_gameMusic = g_theAudioSystem->CreateOrGetSound("Data/Audio/Music/E1M1_AtDoomsGate.mp2");
}

SoundPlaybackID Game::PlaySounds(std::string soundPath)
{
	SoundID NewLevel = g_theAudioSystem->CreateOrGetSound("Data/Audio/NewLevel.mp3");
	return g_theAudioSystem->StartSound(NewLevel);
}
void Game::StopSounds(SoundPlaybackID soundInstance)
{
	g_theAudioSystem->StopSound(soundInstance);
}
void Game::RenderEngineLogo() const
{
	Texture* logoFire;
	Texture* logo;

	AABB2 engineLogoBounds;
	engineLogoBounds.m_mins = m_screenCamera.GetCameraBounds().m_mins + Vec2(400.0f, 200.0f);
	engineLogoBounds.m_maxs = m_screenCamera.GetCameraBounds().m_maxs + Vec2(-400.0f, -200.0f);
	logo = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EngineLogo.png");
	std::vector<Vertex_PCU> screenVerts;
	float engineLogoAlpha = RangeMap(m_engineLogoSeconds - m_currentEngineLogoSeconds, 0.0f, m_engineLogoSeconds, 20.0f, 254.0f);
	int engineLogoAlphaInt = (int)engineLogoAlpha;
	AddVertsForAABB2D(screenVerts, engineLogoBounds, Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), Rgba8(255, 255, 255, (unsigned char)engineLogoAlphaInt));

	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->BindTexture(logo);
	g_theRenderer->DrawVertexArray((int)screenVerts.size(), screenVerts.data());
	screenVerts.clear();
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->EndCamera(m_screenCamera);

	if (m_engineLogoFireSeconds >= 0.0f)
	{
		logoFire = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Explosion_5x5.png");
		SpriteSheet* m_explosionSpriteSheet = new SpriteSheet(*logoFire, IntVec2(5, 5));
		std::vector<Vertex_PCU> explosionVerts;
		SpriteAnimDefinition animation = SpriteAnimDefinition(*m_explosionSpriteSheet, 0, 20, 2.5f, SpriteAnimPlaybackType::ONCE);

		AABB2 explosionAABB2A;
		explosionAABB2A.m_mins = Vec2(6.5f, 4.0f);
		explosionAABB2A.m_maxs = Vec2(7.0f, 4.5f);

		AABB2 explosionAABB2B;
		explosionAABB2B.m_mins = Vec2(9.5f, 4.0f);
		explosionAABB2B.m_maxs = Vec2(10.0f, 4.5f);
		SpriteDefinition currentExplosionSpriteDef = animation.GetSpriteDefAtTime(m_currentEngineLogoSeconds);
		AddVertsForAABB2D(explosionVerts, explosionAABB2A, currentExplosionSpriteDef.GetUVs(), Rgba8(255, 255, 255, 255));
		AddVertsForAABB2D(explosionVerts, explosionAABB2B, currentExplosionSpriteDef.GetUVs(), Rgba8(255, 255, 255, 255));

		g_theRenderer->BeginCamera(m_worldCamera);
		g_theRenderer->SetBlendMode(BlendMode::ADDITIVE);
		g_theRenderer->BindTexture(logoFire);
		g_theRenderer->DrawVertexArray((int)explosionVerts.size(), explosionVerts.data());
		explosionVerts.clear();
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->EndCamera(m_worldCamera);
	}
}
void Game::UpdateEngineLogo(float deltaSeconds)
{
	m_currentEngineLogoSeconds += deltaSeconds;
	m_engineLogoFireSeconds -= deltaSeconds;
	if (m_currentEngineLogoSeconds >= m_engineLogoSeconds)
	{
		SwitchGameModes(GameStates::Attract);
	}
}
void Game::UpdateGameState()
{
	//if (m_PlayerVictory)
	//	SwitchGameModes(GameStates::Victory);
	//if (m_isGameOver)
	//	SwitchGameModes(GameStates::PlayerDead);
}
void Game::SwitchGameModes(GameStates gameState)
{
	if (m_currentGameState != gameState)
	{
		m_currentGameState = gameState;
		if (gameState == GameStates::Attract)
		{
			DeleteMap();
			if ((int)m_players.size() > 0)
			{
				for (int i = 0; i < m_players.size(); i++)
				{
					if (m_players[i])
					{
						delete m_players[i];
						m_players[i] = nullptr;
					}
				}

				m_numberOfPlayersJoined = 0;
			}
			m_players.clear();
			DebugRenderClear();
			g_theAudioSystem->StopSound(m_bgMusicPlayback);
			m_bgMusicPlayback = g_theAudioSystem->StartSound(m_lobbyMusic,true, m_defaultGameSoundVolume);
		}
	}

}
void Game::PlaySoundSfx(SoundPlaybackID soundID)
{
	if (soundID != MISSING_SOUND_ID)
		g_theAudioSystem->StartSound(soundID, false, m_masterVolume);

	else return;

}
bool Game::GetIsMenuScreen()
{
	return m_IsAttractMode;
}
void Game::RenderMenuScreenPlayButton() const
{
	Vertex_PCU tempVertexArrays[3];
	tempVertexArrays[0] = Vertex_PCU(Vec3(5.0f, 0.0f, 0.f), Rgba8(0, 255, 0, (unsigned char)m_playButtonAlpha), Vec2(0.f, 0.f));
	tempVertexArrays[1] = Vertex_PCU(Vec3(-5.0f, -5.0f, 0.f), Rgba8(0, 255, 0, (unsigned char)m_playButtonAlpha), Vec2(0.f, 0.f));
	tempVertexArrays[2] = Vertex_PCU(Vec3(-5.0f, 5.0f, 0.f), Rgba8(0, 255, 0, (unsigned char)m_playButtonAlpha), Vec2(0.f, 0.f));
	TransformVertexArray3D(3, tempVertexArrays, 8.0f, 0.0f, Vec3(WORLD_CENTER_X * 8, WORLD_CENTER_Y * 8, 0.0f));
	g_theRenderer->DrawVertexArray(3, tempVertexArrays);

}
bool Game::GetIsLobbyScreen()
{
	return false;
}
void Game::CreateWorldTextAndBasis()
{
	Vec3 IBasis = Vec3(1.0f, 0.0f, 0.0f);
	Vec3 JBasis = Vec3(0.0f, 1.0f, 0.0f);
	Vec3 KBasis = Vec3(0.0f, 0.0f, 1.0f);
	Mat44 basisMatrix = Mat44(IBasis, JBasis, KBasis, Vec3());
	DebugAddWorldBasis(basisMatrix, -1.0f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::USEDEPTH);

	std::string xbasisText = Stringf("x -forward");
	Mat44 xForwardMatrix = Mat44::CreateXRotationDegrees(90.0f);
	DebugAddWorldText(xbasisText, xForwardMatrix, 0.2f, Vec2(0.0f, 0.0f), -1.0f, Rgba8::RED, Rgba8::RED, DebugRenderMode::ALWAYS);

	std::string ybasisText = Stringf("y -left");
	Mat44 yLeftMatrix = Mat44::CreateZRotationDegrees(90.0f);
	DebugAddWorldText(ybasisText, yLeftMatrix, 0.2f, Vec2(0.0f, 0.0f), -1.0f, Rgba8::GREEN, Rgba8::GREEN, DebugRenderMode::ALWAYS);

	std::string zbasisText = Stringf("z -up");
	Mat44 zUpMatrix = Mat44::CreateYRotationDegrees(-90.0f);
	DebugAddWorldText(zbasisText, zUpMatrix, 0.2f, Vec2(0.0f, 0.0f), -1.0f, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::ALWAYS);
}