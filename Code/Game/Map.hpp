#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include <vector>
#include "Game/Actor.hpp"
#include "Engine/Core/RaycastResult2D.hpp"
#include "Game/SpawnInfo.hpp"
#include "Game/ActorUID.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"


struct RaycastResult
{
	Vec3 m_start = Vec3::ZERO;
	Vec3 m_direction = Vec3::ZERO;
	float m_distance = 0.0f;
	bool m_didImpact = false;
	Vec3 m_impactPosition = Vec3::ZERO;
	Actor* m_impactActor = nullptr;
	bool m_hitFloor = false;
	float m_impactFraction = 0.0f;
	float m_impactDistance = 0.0f;
	Vec3 m_impactSurfaceNormal = Vec3::ZERO;
};
struct RaycastFilter
{
	const Actor* m_ignoreActor = nullptr;
	Faction m_factionToIgnore;
};
//------------------------------------------------------------------------------------------------
class Game;
class MapDefinition;
class Shader;
class Texture;
struct AABB2;
struct Vertex_PCU;
class IndexBuffer;
class VertexBuffer;
class Player;
struct Tile;

//------------------------------------------------------------------------------------------------
class Map
{
public:
	Map(Game* game, const MapDefinition* definition);

	void		Render() const;
	void		Update(float deltaSeconds);
	~Map();
	void CreateTiles();
	void CreateGeometry();
	void CreateBuffers();

	void CreateInitialActors();
	void RenderActors() const;

	RaycastResult RaycastAll(const Vec3& start, const Vec3& direction, float distance, RaycastFilter filter = RaycastFilter()) ;
	RaycastResult RaycastWorldXY(const Vec3& start, const Vec3& direction, float distance, RaycastFilter filter = RaycastFilter());
	RaycastResult RaycastWorldZ(const Vec3& start, const Vec3& direction, float distance, RaycastFilter filter = RaycastFilter()) ;
	RaycastResult RaycastWorldActors(const Vec3& start, const Vec3& direction, float distance, RaycastFilter filter = RaycastFilter()) ;
	RaycastResult RaycastVsActor(const Vec3& start, const Vec3& direction, float distance,Actor* actor);
	Actor* SpawnActor(const SpawnInfo& spawnInfo);
	void DestroyActor(const ActorUID uid);
	Actor* FindActorByUID(const ActorUID uid) const;
	Actor* GetClosestVisibleEnemy(Actor* actor);
	Player* GetPlayer(int playerIndex);
	Player* GetClosestVisiblePlayer(const Actor* actor);
	Player* GetClosestPlayer(const Actor* actor);
	Game* GetGame();
	void SpawnNewActorForPlayer(Player* player);

	//void PossessNextActor();
	void AllActorDestruction();
	void AllActorDeletion();

	void ChangeLightingSettings();
	void AddPointLight(Vec3 Position, float intensity, Rgba8 color, float radius);

	bool CheckIfPointIsWithinMapDimensions(const Vec3& point);
	bool IsPointInsideSolid(const Vec2& point) const;
	bool IsPointInsideSolidIntVec2(const IntVec2& point) const;
	const Tile* GetTileAtPosition(const IntVec2& position) const;
	RaycastResult RaycastVsTiles(Vec2 startingPoint, Vec2 direction, float maxDistance) const;
	Vec3 GetTileImpactNormal(const Vec3& impactPosition);


	IntVec2 m_dimensions;
	std::vector<Actor*> m_actors;
	Game* m_game = nullptr;
	int	 m_currentPlayerRendering = -1;
protected:

	//----------------INFO--------------------------
	const static int m_maxActors = 100000;

	const MapDefinition* m_definition = nullptr;
	std::vector<Tile> m_tiles;

	AABB3 m_bounds;

	//--------------------RENDERING----------------------
	std::vector<Vertex_PNCU> m_vertexes;
	std::vector<unsigned int> m_indexes;
	const Texture* m_texture = nullptr;
	Shader* m_shader = nullptr;
	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
	unsigned int m_indexCount = 0;

	Vec3				m_sunDirection = Vec3(0.0f, 0.0f, -1.0f);
	float				m_sunIntensity = 0.4f;
	float				m_ambientIntensity = 0.8f;
	float				m_sunPitch = 135.0f;
	float				m_sunColor[3];

	//-------------------GAME VARIABLES-----------------
	int m_actorSalt = 0x0000fffe;
};
