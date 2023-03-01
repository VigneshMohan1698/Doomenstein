
#pragma once
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/Player.hpp"
#include "Game/Map.hpp"

enum class GameStates
{
	Attract,
	Lobby,
	GameWorld
};
enum class GameSounds
{
	GamePause = 0,
	GameVictory,
	GameOver,
	TotalSounds
};
enum class InputMode
{
	Keyboard,
	Controller
};
class Game
{
public:
	Game();
	~Game() {}
	//------------------------------MAIN FUNCTIONS------------------------------------------------------
	void				Startup();
	void				Update(float deltaSeconds);
	void				Render() const;
	void				EndFrame();
	void				ShutDown();
	//------------------------------MENU SCREEN FUNCTIONS-----------------------------------------------
	void				RenderAttractMode() const;
	void				UpdateAttractMode(float deltaSeconds);
	bool				GetIsMenuScreen();
	void				RenderMenuScreenPlayButton() const;

	//-----------------------------LOBBY SCREEN FUNCTIONS----------------------------------------------
	void				RenderLobby () const;
	void				UpdateLobby(float deltaSeconds);
	void				SetCamerasForPlayer(Player* player);
	bool				GetIsLobbyScreen();
	void				SwitchToLobby(InputMode mode, int controllerId);
	void				CheckPlayersBackingOut();
	void				CheckPlayersSwitchingToGame();
	bool				CheckIfControllerJoined(int controllerIndex);

	//------------------------------GAME FUNCTIONS-------------------------------------------------------
	void				GetMouseInputAndSpawnRaycasts();
	void				DebugRender() const;
	void				SetupInitialVariables();
	void				UpdateGameState();
	void				SwitchGameModes(GameStates gameState);
	void				UpdateGame(float deltaSeconds);

	void				SetWindowSettings();
	void				CheckPlayerDeadAndRespawn();
	void				UpdateWorldCameraFov(float fov);
	void				RenderGame() const;
	void				CreateWorldTextAndBasis();

	void				InitializeCameras();
	void				PlaySoundSfx(SoundPlaybackID soundID);
	void				UpdateEngineLogo(float deltaSeconds);
	void				RenderEngineLogo() const;
	void				LoadGameSounds();
	SoundPlaybackID		PlaySounds(std::string soundPath);
	void				StopSounds(SoundPlaybackID soundInstance);
	void				SwitchToGame(float deltaSeconds);
	void				DrawDebugScreenTexts(float deltaSeconds);
	void				PauseGame(bool pauseGame);
	Camera				GetWorldCamera();
	Camera				m_worldCamera;
	Camera				m_screenCamera;

	//--------------------------------DOOMENSTEIN MAP/TILE FUNCTIONS ------------------------------------
	void				CreateMap();
	void				DeleteMap();
	void				ClampPlayerWithinMapBounds();
public:
	static const  int	m_maxPlayers = 2;
	bool				m_isPlayerDead = false;
	Map*				m_currentMap = nullptr;

	std::vector<Player*> m_players;
	int					m_numberOfPlayersJoined = 0;

	GameStates			m_currentGameState = GameStates::Attract;
	SoundID				m_gameSounds[(int)GameSounds::TotalSounds] = {};
	bool				m_PlayerVictory = false;
	bool				m_isGameOver = false;
	bool				m_isGamePaused = false;
	float				m_masterVolume = 1.0f;
	float				m_currentEngineLogoSeconds = 0.0f;
	float				m_engineLogoFireSeconds = 2.0f;
	float				m_engineLogoSeconds = 3.5f;
	Vec2				m_mouseSensitivity  = Vec2(0.1f, 0.1f);
	float				m_playerVelocity = 1.0f;
private:
	std::vector<Vertex_PCU> m_deadBox;
	bool				m_IsAttractMode = false;
	float			    m_playButtonAlpha = 255.0f;
	Vec2				m_cursorPosition;			
	std::string			m_AttractScreenMusic;
	std::string			m_GameMusic;
	float				m_defaultGameSoundVolume = 0.0f;
	SoundID				m_lobbyMusic = MISSING_SOUND_ID;
	SoundID				m_gameMusic = MISSING_SOUND_ID;
	SoundPlaybackID     m_bgMusicPlayback = MISSING_SOUND_ID;
};