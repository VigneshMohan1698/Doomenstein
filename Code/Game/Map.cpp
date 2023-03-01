#include "Map.hpp"
#include "Engine/Core/Image.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/TileSetDefinition.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/VertexUtils.hpp"
#include "Engine/Renderer/SpriteSheet.cpp"
#include "Game/Tile.hpp"
#include "Game/TileMaterialDefinition.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include <d3d11.h>
#include "Game/Player.hpp"
#include <Engine/Core/ErrorWarningAssert.hpp>
#include "Game/AI.hpp"
#include "Game/Game.hpp"
#include "Engine/Input/InputSystem.hpp"
#include <Engine/Renderer/DebugRenderer.hpp>

extern InputSystem* g_theInputSystem;
extern Renderer* g_theRenderer;
Map::Map(Game* game, const MapDefinition* definition)
{
	m_game = game;
	m_definition = definition;
	m_dimensions.x = definition->m_image.GetDimensions().x;
	m_dimensions.y = definition->m_image.GetDimensions().y;
	m_shader = g_theRenderer->CreateOrGetShader("Data/Shaders/SpriteLit");
	AddPointLight(Vec3(30.5f, 15.5f, 0.5f), 1.0f, Rgba8(1.0f,1.0f,1.0f,1.0f), 3.0f);
}
void Map::Render() const
{
	Texture* texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png");
	g_theRenderer->BindTexture(texture);
	g_theRenderer->BindShader(m_shader);
	g_theRenderer->BindVertexBuffer(m_vertexBuffer);
	g_theRenderer->BindIndexBuffer(0,m_indexBuffer);
	g_theRenderer->DrawIndexedVertexArray((int)m_vertexes.size(), m_vertexes.data(), (int)m_indexes.size(), m_indexes.data(), m_vertexBuffer, m_indexBuffer);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader(nullptr);
	RenderActors();
}
void Map::RenderActors() const
{
	for (int i = 0; i < (int)m_actors.size(); i++)
	{
		if (m_actors[i])
		{
			if (!m_actors[i]->m_isDestroyed)
			{
				m_actors[i]->Render();
			}
		}
	}
}
void Map::Update(float deltaSeconds)
{
	UNUSED((void)deltaSeconds);
	
	for (int i = 0; i < (int)m_actors.size(); i++)
	{
		if(m_actors[i])
			m_actors[i]->Update(deltaSeconds);
	}
	ChangeLightingSettings();
	AllActorDestruction();
}
Map::~Map()
{
	m_tiles.clear();
	if (m_vertexBuffer)
	{
		DX_SAFE_RELEASE(m_vertexBuffer->m_buffer);
	}
	if (m_indexBuffer)
	{
		DX_SAFE_RELEASE(m_indexBuffer->m_buffer);
	}
	for (int i = 0; i < (int)GetGame()->m_players.size(); i++)
	{
		g_theRenderer->ReleaseSampler(GetGame()->m_players[i]->m_samplerState);
		g_theRenderer->ReleaseSampler(GetGame()->m_players[i]->m_samplerState2);
	}

}

void Map::CreateTiles()
{
	RandomNumberGenerator rng;
	const TileSetDefinition* tileSet = m_definition->m_tileSetDefinition;
	Image mapImage					 = m_definition->m_image;
	AABB3 tileBounds				 = AABB3();

	for (int j = 0; j < mapImage.GetDimensions().x; j++)
	{
 		for (int i = 0; i < mapImage.GetDimensions().y; i++)
		{
			tileBounds = AABB3(Vec3((float)j, (float)i,0.0f),Vec3(j+1.0f,i+1.0f,1.0f));
			const TileDefinition* tileDefinition;
			Rgba8 texlColor			 = mapImage.GetTexelColor(IntVec2(i, j));
			//int randominteger = rng.GetRandomIntInRange(0, 255);
			//if (texlColor.a == 0)
			//{
			//	continue;
			//}
			//else 
			{
				tileDefinition		 = tileSet->GetTileDefinitionByColor(texlColor);
				if (tileDefinition->m_name != "Air")
				{
					Tile tile = Tile(tileBounds, tileDefinition);
					m_tiles.push_back(tile);
				}
			}
		}
	}
	CreateGeometry();
}
void Map::CreateGeometry()
{
	Vec3 bottomLeft, bottomRight, topRight, topLeft;
	float minx, miny, minz;
	float maxx, maxy, maxz;
	const TileMaterialDefinition* material = nullptr;
	for (int i = 0; i < (int)m_tiles.size(); i++)
	{
		material = m_tiles[i].m_definition->m_floorMaterialDefinition;
		if (material)
		{
 			SpriteSheet* m_mapSpriteSheet = new SpriteSheet(*material->m_texture, IntVec2(8, 8));

			minx = m_tiles[i].m_bounds.m_mins.x;
			miny = m_tiles[i].m_bounds.m_mins.y;
			minz = m_tiles[i].m_bounds.m_mins.z;

			maxx = m_tiles[i].m_bounds.m_maxs.x;
			maxy = m_tiles[i].m_bounds.m_maxs.y;
			maxz = m_tiles[i].m_bounds.m_maxs.z;

			bottomLeft = Vec3(minx, maxy, minz);
			bottomRight = Vec3(minx, miny, minz);
			topLeft = Vec3(maxx, maxy, minz);
			topRight = Vec3(maxx, miny, minz);

			const SpriteDefinition& spriteDefinition = m_mapSpriteSheet->GetSpriteDef(material->m_uv.m_mins, IntVec2(8,8));
			AddVertsForIndexedQuad3D(m_vertexes, m_indexes,m_indexCount, topLeft, bottomLeft, bottomRight, topRight, Rgba8::WHITE, spriteDefinition.GetUVs());
			m_indexCount += 4;
		}

	    material = m_tiles[i].m_definition->m_ceilingMaterialDefinition;
		if (material)
		{
			SpriteSheet* m_mapSpriteSheet = new SpriteSheet(*material->m_texture, IntVec2(8, 8));

			minx = m_tiles[i].m_bounds.m_mins.x;
			miny = m_tiles[i].m_bounds.m_mins.y;
			minz = m_tiles[i].m_bounds.m_mins.z;

			maxx = m_tiles[i].m_bounds.m_maxs.x;
			maxy = m_tiles[i].m_bounds.m_maxs.y;
			maxz = m_tiles[i].m_bounds.m_maxs.z;

			bottomLeft = Vec3(maxx, maxy, maxz);
			bottomRight = Vec3(maxx, miny, maxz);
			topLeft = Vec3(minx, maxy, maxz);
			topRight = Vec3(minx, miny, maxz);

			const SpriteDefinition& spriteDefinition = m_mapSpriteSheet->GetSpriteDef(material->m_uv.m_mins, IntVec2(8, 8));
			AddVertsForIndexedQuad3D(m_vertexes, m_indexes, m_indexCount, topLeft, bottomLeft, bottomRight, topRight, Rgba8::WHITE, spriteDefinition.GetUVs());
			m_indexCount += 4;
		}

		material = m_tiles[i].m_definition->m_wallMaterialDefinition;
		if (material)
		{
			SpriteSheet* m_mapSpriteSheet = new SpriteSheet(*material->m_texture, IntVec2(8, 8));

			minx = m_tiles[i].m_bounds.m_mins.x;
			miny = m_tiles[i].m_bounds.m_mins.y;
			minz = m_tiles[i].m_bounds.m_mins.z;

			maxx = m_tiles[i].m_bounds.m_maxs.x;
			maxy = m_tiles[i].m_bounds.m_maxs.y;
			maxz = m_tiles[i].m_bounds.m_maxs.z;

			//----------------------RIGHT WALL-------------------------------

			bottomLeft = Vec3(maxx, maxy, minz);
			bottomRight = Vec3(minx, maxy, minz);
			topLeft = Vec3(maxx, maxy, maxz);
			topRight = Vec3(minx, maxy, maxz);

			const SpriteDefinition& spriteDefinition = m_mapSpriteSheet->GetSpriteDef(material->m_uv.m_mins, IntVec2(8, 8));
			AddVertsForIndexedQuad3D(m_vertexes, m_indexes, m_indexCount, topLeft, bottomLeft, bottomRight, topRight, Rgba8::WHITE, spriteDefinition.GetUVs());
			m_indexCount += 4;

			//-------------------------LEFT WALL-----------------------------
			bottomLeft = Vec3(minx, miny, minz);
			bottomRight = Vec3(maxx, miny, minz);
			topLeft = Vec3(minx, miny, maxz);
			topRight = Vec3(maxx, miny, maxz);
			AddVertsForIndexedQuad3D(m_vertexes, m_indexes, m_indexCount, topLeft, bottomLeft, bottomRight, topRight, Rgba8::WHITE, spriteDefinition.GetUVs());
			m_indexCount += 4;

			//------------------------FRONT WALL---------------------------
			bottomLeft = Vec3(minx, maxy, minz);
			bottomRight = Vec3(minx, miny, minz);
			topLeft = Vec3(minx, maxy, maxz);
			topRight = Vec3(minx, miny, maxz);
			AddVertsForIndexedQuad3D(m_vertexes, m_indexes, m_indexCount, topLeft, bottomLeft, bottomRight, topRight, Rgba8::WHITE, spriteDefinition.GetUVs());
			m_indexCount += 4;

			//----------------------BACK WALL------------------------------
			bottomLeft = Vec3(maxx, miny, minz);
			bottomRight = Vec3(maxx, maxy, minz);
			topLeft = Vec3(maxx, miny, maxz);
			topRight = Vec3(maxx, maxy, maxz);
			AddVertsForIndexedQuad3D(m_vertexes, m_indexes, m_indexCount, topLeft, bottomLeft, bottomRight, topRight, Rgba8::WHITE, spriteDefinition.GetUVs());
			m_indexCount += 4;
		}
	}
	CreateBuffers();
	
}
void Map::CreateBuffers()
{
	size_t size = sizeof(Vertex_PNCU) * m_vertexes.size();
	m_vertexBuffer = new VertexBuffer(size, sizeof(Vertex_PNCU));
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(size, m_vertexBuffer);
	g_theRenderer->CopyCPUToGPU(m_vertexes.data(), size, m_vertexBuffer);

	size = sizeof(unsigned int) * m_indexes.size();
	m_indexBuffer = new IndexBuffer(size);
	m_indexBuffer = g_theRenderer->CreateIndexBuffer(size, m_indexBuffer);
	g_theRenderer->CopyCPUToGPU(m_indexes.data(), size, m_indexBuffer);

}
void Map::CreateInitialActors()
{
	RandomNumberGenerator rng;
	std::vector<SpawnInfo> randomMarineSpawnPoint;
	std::vector<SpawnInfo> demonSpawnPoint;
	std::vector<SpawnInfo> spawnPoint;
	SpawnInfo marineSpawn;
	SpawnInfo demonSpawn;
	for (int i = 0; i < (int)m_definition->m_spawnInfos.size(); i++)
	{
		if (m_definition->m_spawnInfos[i].m_definition->m_name == "SpawnPoint")
		{
			spawnPoint.push_back(m_definition->m_spawnInfos[i]);
		}
	}

	for (int i = 0; i < (int)m_definition->m_spawnInfos.size(); i++)
	{
		if (m_definition->m_spawnInfos[i].m_definition->m_name == "FlyingDemon" || m_definition->m_spawnInfos[i].m_definition->m_name == "Demon")
		{
			demonSpawnPoint.push_back(m_definition->m_spawnInfos[i]);
		}
	}
	
	for (int i = 0; i < (int)GetGame()->m_players.size(); i++)
	{
		/*int randomSpawnPoint = rng.GetRandomIntInRange(0, (int)randomMarineSpawnPoint.size() - 1);*/
		marineSpawn = spawnPoint[i];
		SpawnActor(marineSpawn);
		GetPlayer(i)->Possess(m_actors[i]);
		GetPlayer(i)->m_position = m_actors[i]->m_position;
		m_actors[i]->m_controller = GetPlayer(i);
		GetPlayer(i)->SetMap(this);
	}

	for (int i = 0; i < (int)demonSpawnPoint.size(); i++)
	{
		SpawnActor(demonSpawnPoint[i]);
	}
}

RaycastResult Map::RaycastAll(const Vec3& start, const Vec3& direction, float distance, RaycastFilter filter)
{
	RaycastResult result1, result2, result3,finalResult;
	result1 = RaycastWorldZ(start, direction, distance,filter);
	result2 = RaycastWorldXY(start, direction, distance, filter);
	result3 = RaycastWorldActors(start, direction, distance, filter);

	((result1.m_impactDistance < result2.m_impactDistance && result1.m_didImpact) || (!result2.m_didImpact && result1.m_didImpact)) ? finalResult = result1 : finalResult = result2;
	((result3.m_impactDistance < finalResult.m_impactDistance && result3.m_didImpact) || (!finalResult.m_didImpact && result3.m_didImpact)) ? finalResult = result3 : finalResult = finalResult;

	return finalResult;
}
RaycastResult Map::RaycastWorldXY(const Vec3& start, const Vec3& direction, float distance, RaycastFilter filter)
{
	UNUSED((void)filter);
	RaycastResult result;
	Vec3 end = start + (direction * distance);
	Vec3 newEnd;
	if ((start.z > 1.0f && end.z > 1.0f) || (start.z < 0.0f && end.z < 0.0f))
	{
		result.m_didImpact = false;
		return result;
	}
	result = RaycastVsTiles(Vec2(start), Vec2(direction.x, direction.y).GetNormalized(), distance);
	float length, length2,t;
	length = GetDistance2D(Vec2(start), Vec2(end));
	length2 = result.m_impactDistance;
	t = (length2 * distance) / length;
	newEnd = start + (direction * t);
	result.m_impactPosition = newEnd;
	result.m_impactSurfaceNormal.z = result.m_impactPosition.z;

	if (!CheckIfPointIsWithinMapDimensions(result.m_impactPosition ))
	{
		result.m_didImpact = false;
		return result;
	}
	
	result.m_impactDistance = GetDistance3D(start, result.m_impactPosition);
	return result;
}
RaycastResult Map::RaycastWorldZ(const Vec3& start, const Vec3& direction, float distance, RaycastFilter filter)
{
	UNUSED((void)filter);
	RaycastResult returnValue;
	float mapStartZ, mapEndZ;
	mapStartZ = 0.0f;
	mapEndZ = 1.0f;
	float t,deltaZ,directionZ;
	Vec3 end = start + (direction * distance);
	Vec3 newEnd;
	directionZ = fabsf(direction.z);
	if (end.z >= mapEndZ)
	{
		deltaZ = fabsf(start.z - 1.0f);
		t = (deltaZ / directionZ);
		newEnd = start + (direction * t);
		returnValue.m_impactPosition = newEnd;
		returnValue.m_didImpact = CheckIfPointIsWithinMapDimensions(returnValue.m_impactPosition);
		returnValue.m_impactDistance = GetDistance3D(start, returnValue.m_impactPosition);
		returnValue.m_impactSurfaceNormal = Vec3(newEnd.x, newEnd.y, newEnd.z - 0.3f);
	}
	if (end.z <= mapStartZ)
	{
		//t = RangeMap(mapStartZ, start.z, end.z, 0.0f, distance);
		deltaZ = fabsf(start.z);
		t = (deltaZ / directionZ);
		newEnd = start + (direction * t);
		returnValue.m_impactPosition = newEnd;
		returnValue.m_didImpact = CheckIfPointIsWithinMapDimensions(returnValue.m_impactPosition);
		returnValue.m_impactDistance = GetDistance3D(start, returnValue.m_impactPosition);
		returnValue.m_impactSurfaceNormal = Vec3(newEnd.x, newEnd.y, newEnd.z + 0.3f);
	}
	if (!CheckIfPointIsWithinMapDimensions(returnValue.m_impactPosition))
	{
		returnValue.m_didImpact = false;
		return returnValue;
	}
	else if (returnValue.m_impactDistance > distance)
	{
		returnValue.m_didImpact = false;
		return returnValue;
	}
	return returnValue;
}
RaycastResult Map::RaycastWorldActors(const Vec3& start, const Vec3& direction, float distance, RaycastFilter filter )
{
	RaycastResult result, finalResult;
	for (int i = 0; i < (int)m_actors.size(); i++)
	{
		if (m_actors[i] == nullptr)
		{
			continue;
		}
		if (m_actors[i] != filter.m_ignoreActor || m_actors[i]->m_definition->m_faction != filter.m_factionToIgnore )
		{
			result = RaycastVsActor(start, direction, distance, m_actors[i]);
		}
		if (result.m_didImpact)
		{
			float thisLoopHit = result.m_impactDistance;
			float lastLoopHit = finalResult.m_impactDistance;

			if (finalResult.m_impactPosition == Vec3::ZERO || thisLoopHit < lastLoopHit)
			{
				finalResult = result;
			}
		}
	}
	return finalResult;
}
RaycastResult Map::RaycastVsActor(const Vec3& start, const Vec3& direction, float distance,Actor* actor)
{
	RaycastResult returnValue;
	Vec3 end = start + (direction * distance);
	float height, radius;
	Vec3 actorPosition = actor->m_position;
	height = actor->m_physicsHeight;
	radius = actor->m_physicsRadius;
	Vec3 cylinderTop = Vec3(actor->m_position.x, actor->m_position.y, actor->m_position.z + height);
	Vec3 cylinderBottom = actor->m_position;
	float cylinderTopZ = cylinderTop.z;
	float cylinderBottomZ = cylinderBottom.z;

	float t = 0.0f, length2, length;
	length = GetDistance2D(Vec2(start), Vec2(end));
	RaycastResult2D discImpact = RaycastVsDisc2D(Vec2(start.x,start.y), Vec2(direction.x,direction.y).GetNormalized(), distance, Vec2(actorPosition.x, actorPosition.y), radius);
	Vec3 impactPoint;
	if (discImpact.didImpact)
	{
		//-------------------------------------CHECKING CYLINDER SIDE COLLISIONS----------------------------------

		length2 = discImpact.impactDistance;
		t = (length2 * distance) / length;
		returnValue.m_impactPosition = start + (direction*t);

		float deltaZ,directionZ;
		directionZ = fabsf(direction.z);
		//------------------------------------CHECKING IF RAY START IS INSIDE CYLINDER----------------------------------
		if (returnValue.m_impactPosition == start)
		{
			returnValue.m_didImpact = true;
			returnValue.m_impactDistance = 0.0f;
			returnValue.m_impactSurfaceNormal = returnValue.m_impactPosition + (direction * -0.3f);
			if (returnValue.m_didImpact)
			{
				returnValue.m_impactActor = actor;
			}
			return returnValue;
		}

		//------------------------------------CHECKING TOP CIRCLE COLLISIONS----------------------------------
		else if (returnValue.m_impactPosition.z >= cylinderTopZ && directionZ != 0.0f)
		{
			
			deltaZ = start.z - cylinderTopZ;
			t = (deltaZ / directionZ );
			returnValue.m_impactPosition = start + (direction * t);
			if (IsPointInsideDisc(Vec2(cylinderTop), Vec2(returnValue.m_impactPosition), radius))
			{
				returnValue.m_didImpact = true;
				returnValue.m_impactSurfaceNormal = returnValue.m_impactPosition;
				returnValue.m_impactSurfaceNormal.z += 0.3f;
				returnValue.m_impactDistance = GetDistance3D(start, returnValue.m_impactPosition);
		    }

		}
		//------------------------------------CHECKING BOTTOM CIRCLE COLLISIONS----------------------------------
		else if(returnValue.m_impactPosition.z <= cylinderBottomZ && directionZ != 0.0f)
		{
			deltaZ = cylinderBottomZ - start.z;
			t = (deltaZ / directionZ);
			returnValue.m_impactPosition = start + (direction * t);
			if (IsPointInsideDisc(Vec2(cylinderBottom), Vec2(returnValue.m_impactPosition), radius))
			{
				returnValue.m_didImpact = true;
				returnValue.m_impactSurfaceNormal = returnValue.m_impactPosition;
				returnValue.m_impactSurfaceNormal.z -= 0.3f;
				returnValue.m_impactDistance = GetDistance3D(start, returnValue.m_impactPosition);
			}
		}
		//--------------------------------------SIDE COLLISION FINAL ----------------------------
		else
		{
			returnValue.m_didImpact = true;
			Vec3 normalCenter = actorPosition;
			normalCenter.z = returnValue.m_impactPosition.z;
			Vec3 normal;
			normal = (normalCenter - returnValue.m_impactPosition).GetNormalized();
			returnValue.m_impactSurfaceNormal = returnValue.m_impactPosition - (0.3f * normal);
			returnValue.m_impactDistance = GetDistance3D(start, returnValue.m_impactPosition);
		}
	}
	if (returnValue.m_didImpact )
	{
		returnValue.m_impactActor = actor;
	}

	return returnValue;
}

Actor* Map::SpawnActor(const SpawnInfo& spawnInfo)
{
	if ((int)m_actors.size() >= m_maxActors)
	{
		ERROR_AND_DIE("Max actor limit reached");
	}
	m_actorSalt++;
	if (m_actorSalt > 0x0000ffe)
	{
		m_actorSalt = 0;
	}
	int unusedIndex = -1;
	for (int i = 0; i < m_actors.size(); i++)
	{
		if (m_actors[i] == nullptr)
		{
			unusedIndex = i;
			break;
		}
	}

	Actor* actor = new Actor(this, spawnInfo);
	const ActorDefinition* actorDefinition = nullptr;
	if (spawnInfo.m_definition->m_name == "SpawnPoint")
	{
		actorDefinition = nullptr;
		actorDefinition = ActorDefinition::GetByName("Marine");
		actor->m_definition = actorDefinition;
		actor->m_physicsHeight = actorDefinition->m_physicsHeight;
		actor->m_physicsRadius = actorDefinition->m_physicsRadius;
	}
	else
	{
		actorDefinition = spawnInfo.m_definition;
		actor->m_definition = actorDefinition;
		actor->m_physicsHeight = actorDefinition->m_physicsHeight;
		actor->m_physicsRadius = actorDefinition->m_physicsRadius;

	}
	actor->CreateWeaponsFromInventory();
	actor->AddVertsForActor();
	if (unusedIndex == -1)
	{
		ActorUID uid = ActorUID((int)m_actors.size(), m_actorSalt);
		actor->m_uid = uid;
		m_actors.push_back(actor);
	}
	else
	{
		ActorUID uid = ActorUID(unusedIndex, m_actorSalt);
		actor->m_uid = uid;
		m_actors[unusedIndex] = actor;
	}
	if (actorDefinition->m_name == ("Demon") || actorDefinition->m_name == "FlyingDemon")
	{
		actor->m_aiController = new AI();
		actor->m_aiController->Possess(actor);
		actor->m_controller = actor->m_aiController;
		actor->m_aiController->SetMap(this);
	}
	return actor;
}
void Map::DestroyActor(const ActorUID uid)
{
	Actor* actor = FindActorByUID(uid);
	if (actor == nullptr)
	{
		return;
	}
	delete actor;
	int index = uid.GetIndex();
	m_actors[index] = nullptr;
}
Actor* Map::FindActorByUID(const ActorUID uid) const
{
	if (uid == ActorUID::INVALID)
	{
		return nullptr;
	}
	int index = uid.GetIndex();
	if (m_actors[index] == nullptr)
	{
		return nullptr;
	}
	if (m_actors[index]->m_uid == uid)
	{
		return m_actors[index];
	}
	return nullptr;
}
Actor* Map::GetClosestVisibleEnemy(Actor* actor)
{
	UNUSED((void)actor);
	return nullptr;
}
Player* Map::GetPlayer(int playerIndex)
{
	if (playerIndex > (int)m_game->m_players.size() -1 )
	{
		return nullptr;
	}
	return m_game->m_players[playerIndex];
}
Player* Map::GetClosestVisiblePlayer(const Actor* actor)
{
	Player* player = nullptr;
	Actor* playerActor = nullptr;
	float sightRadius = actor->m_definition->m_sightRadius;
	float sightAngle = actor->m_definition->m_sightAngle;
	float goalOrientationYaw;
	float minimumDistance = 1000.0f;
	int currentClosestPlayerIndex = -1;
	for (int i = 0; i < (int)m_game->m_players.size(); i++)
	{
		player = m_game->m_players[i];
		playerActor = player->GetActor();
		if (playerActor == actor || playerActor == nullptr)
		{
			continue;
		}
		
		Vec3 playerPosition = player->m_position;
		Vec3 direction = playerPosition - actor->m_position;
		RaycastFilter filter;
		filter.m_ignoreActor = actor;
		if (direction.GetLength() != 0.0f)
		{
			RaycastResult result = RaycastVsActor(actor->m_position, direction.GetNormalized(), 100.0f, playerActor);
			RaycastResult resultvsWalls = RaycastWorldXY(actor->m_position, direction.GetNormalized(), 100.0f);
			if (!resultvsWalls.m_didImpact || (result.m_didImpact && result.m_impactDistance < resultvsWalls.m_impactDistance))
			{
				Vec2 impactAnglePosition = Vec2(result.m_impactPosition) - Vec2(actor->m_position);
				goalOrientationYaw = impactAnglePosition.GetOrientationDegrees();
				float angleDelta = GetAngleDegreesBetweenVectors2D(impactAnglePosition, Vec2(actor->GetForward()).GetNormalized());
				if ((result.m_impactDistance > sightRadius) || angleDelta > sightAngle / 2.0f)
				{
					player = nullptr;
					playerActor = nullptr;
					continue;
				}
				else
				{
					if (result.m_impactDistance < minimumDistance)
					{
						minimumDistance = result.m_impactDistance;
						currentClosestPlayerIndex = i;
					}
				}
			}
		}
	}
	if (currentClosestPlayerIndex == -1)
	{
		return nullptr;
	}
	else
	{
		return m_game->m_players[currentClosestPlayerIndex];
	}

}
Player* Map::GetClosestPlayer(const Actor* actor)
{
	Player* closestVisiblePlayer = GetClosestVisiblePlayer(actor);
	if (closestVisiblePlayer)
	{
		return closestVisiblePlayer;
	}
	float max  = 100000.0f;
	float distance  = 0.0f;
	for (int i = 0; i < (int)m_game->m_players.size(); i++)
	{
		distance = GetDistance3D(m_game->m_players[i]->GetActor()->m_position, actor->m_position);
		if (distance < max && m_game->m_players[i]->GetActor() != actor)
		{
			closestVisiblePlayer = m_game->m_players[i];
			max = distance;
		}
	}
	return closestVisiblePlayer;
}
Game* Map::GetGame()
{
	return m_game;
}
//void Map::PossessNextActor()
//{
//	Actor* currentPossessedActor = GetGame()->m_player->GetActor();
//	int index = 0;
//	int nextIndex = 0;
//	bool actorfound = true;
//	int currentActorIndex = currentPossessedActor->m_uid.GetIndex();
//	while (actorfound)
//	{
//		index++;
//		nextIndex = currentActorIndex + index;
//		if (nextIndex >= (int)m_actors.size())
//		{
//			index = 0;
//			currentActorIndex = 0;
//			nextIndex = 0;
//		}
//		if (m_actors[nextIndex] != nullptr)
//		{
//			if (m_actors[nextIndex]->m_definition->m_canBePossessed)
//			{
//				if (GetPlayer()->GetActor())
//				{
//					GetPlayer()->GetActor()->m_controller = GetPlayer()->GetActor()->m_aiController;
//				}
//				GetPlayer()->Possess(m_actors[nextIndex]);
//				GetPlayer()->m_position = m_actors[nextIndex]->m_position;
//				GetPlayer()->m_position.z = m_actors[nextIndex]->m_definition->m_eyeHeight;
//				m_actors[nextIndex]->m_controller = GetPlayer();
//				m_game->UpdateWorldCameraFov(m_actors[nextIndex]->m_definition->m_cameraFOVDegrees);
//				actorfound = false;
//			}
//		}
//	}
//	return;
//}

bool Map::CheckIfPointIsWithinMapDimensions(const Vec3& point)
{
	if (point.x <= m_dimensions.x && point.x >= 0.0f && point.y <= m_dimensions.y && point.y >=0.0f)
	{
		if (point.z >= 0.0f && point.z <= 1.0f)
		{
			return true;
		}
	}
	return false;
}
bool Map::IsPointInsideSolid(const Vec2& point) const
{
	IntVec2 tilePos = IntVec2(RoundDownToInt(point.x), RoundDownToInt(point.y));
	if (GetTileAtPosition(tilePos) != nullptr)
	{
		if (GetTileAtPosition(tilePos)->IsSolid())
		{
			return true;
		}
	}
	return false;
}
bool Map::IsPointInsideSolidIntVec2(const IntVec2& point) const
{
	if (GetTileAtPosition(point) != nullptr)
	{
		if (GetTileAtPosition(point)->IsSolid())
		{
			return true;
		}
	}
	return false;
}
const Tile* Map::GetTileAtPosition(const IntVec2& position) const
{
	for (int i = 0; i < m_tiles.size(); i++)
	{
		if (m_tiles[i].m_bounds.m_mins == Vec3((float)position.x, (float)position.y, 0.0f))
		{
			return &m_tiles[i];
		}
	}
	return nullptr;
}
RaycastResult Map::RaycastVsTiles(Vec2 startingPoint, Vec2 direction, float maxDistance) const
{
	RaycastResult result;
	Vec2 endPoint = startingPoint + maxDistance * direction;
	IntVec2 tileCoords = IntVec2(RoundDownToInt(startingPoint.x), RoundDownToInt(startingPoint.y));

	if (IsPointInsideSolid(startingPoint))
	{
		result.m_impactPosition = Vec3(startingPoint.x, startingPoint.y,0.0f);
		result.m_didImpact = true;
		result.m_impactDistance = (Vec2(result.m_impactPosition) - startingPoint).GetLength();
		return result;
	}
	float fwdDistPerXCrossing = 1.0f / fabsf(direction.x);
	float fwdDistPerYCrossing = 1.0f / fabsf(direction.y);

	int tileStepDirectionX;
	int tileStepDirectionY;

	if (direction.x < 0)
		tileStepDirectionX = -1;
	else
		tileStepDirectionX = 1;

	float xAtFirstCrossing = (float)tileCoords.x + (tileStepDirectionX + 1) / 2.0f;
	float xDistanceToFirstCrossing = xAtFirstCrossing - startingPoint.x;

	float fwdDistanceAtNextXCrossing = fabsf(xDistanceToFirstCrossing) * fwdDistPerXCrossing;
	//-----------x done -----------------

	if (direction.y < 0)
		tileStepDirectionY = -1;
	else
		tileStepDirectionY = 1;

	float yAtFirstCrossing = (float)tileCoords.y + (tileStepDirectionY + 1) / 2.0f;
	float yDistanceToFirstCrossing = yAtFirstCrossing - startingPoint.y;

	float fwdDistanceAtNextYCrossing = fabsf(yDistanceToFirstCrossing) * fwdDistPerYCrossing;

	while (true)
	{
		if (fwdDistanceAtNextXCrossing <= fwdDistanceAtNextYCrossing)
		{
			if (fwdDistanceAtNextXCrossing > maxDistance)
			{
				result.m_didImpact = false;
				return result;
			}

			tileCoords.x += tileStepDirectionX;
			if (IsPointInsideSolidIntVec2(tileCoords))
			{
				endPoint = startingPoint + (direction * fwdDistanceAtNextXCrossing);
				result.m_didImpact = true;
				result.m_impactPosition = Vec3(endPoint.x,endPoint.y, 0.0f);
				result.m_impactDistance = (Vec2(result.m_impactPosition) - startingPoint).GetLength();
				result.m_impactSurfaceNormal = result.m_impactPosition;
				if (startingPoint.x <= result.m_impactPosition.x)
				{
					result.m_impactSurfaceNormal.x = result.m_impactPosition.x - 0.3f;
				}
				else
				{
					result.m_impactSurfaceNormal.x = result.m_impactPosition.x + 0.3f;
				}
				return result;
			}

			fwdDistanceAtNextXCrossing += fwdDistPerXCrossing;
		}
		else if (fwdDistanceAtNextYCrossing < fwdDistanceAtNextXCrossing)
		{
			if (fwdDistanceAtNextYCrossing > maxDistance)
			{
				result.m_didImpact = false;
				return result;
			}

			tileCoords.y += tileStepDirectionY;
			if (IsPointInsideSolidIntVec2(tileCoords))
			{
				endPoint = startingPoint + (direction * fwdDistanceAtNextYCrossing);
				result.m_didImpact = true;
				result.m_impactPosition = Vec3(endPoint.x,endPoint.y, 0.0f);
				result.m_impactDistance = (Vec2(result.m_impactPosition) - startingPoint).GetLength();
				result.m_impactSurfaceNormal = result.m_impactPosition;
				if (startingPoint.y <= result.m_impactPosition.y)
				{
					result.m_impactSurfaceNormal.y = result.m_impactPosition.y - 0.3f;
				}
				else
				{
					result.m_impactSurfaceNormal.y = result.m_impactPosition.y + 0.3f;
				}
				return result;
			}

			fwdDistanceAtNextYCrossing += fwdDistPerYCrossing;
		}

	}


}
Vec3 Map::GetTileImpactNormal(const Vec3& impactPosition)
{
	Vec3 surfaceNormal;
	const Tile* tile = GetTileAtPosition(IntVec2(Vec2(impactPosition)));
	AABB3 bounds = tile->m_bounds;
	if (impactPosition.x >= bounds.m_mins.x)
	{
		surfaceNormal = Vec3(0.1f, 0.0f, 0.0f);
	}
	else if (impactPosition.x <= bounds.m_maxs.x)
	{
		surfaceNormal = Vec3(-0.1f, 0.0f, 0.0f);
	}
	else if (impactPosition.y >= bounds.m_mins.y)
	{
		surfaceNormal = Vec3(0.0f, 0.1f, 0.0f);
	}
	else if (impactPosition.y <= bounds.m_maxs.y)
	{
		surfaceNormal = Vec3(0.0f, -0.1f, 0.0f);
	}
	return surfaceNormal;
}

void Map::SpawnNewActorForPlayer(Player* player)
{
	RandomNumberGenerator rng;
	std::vector<SpawnInfo> spawnPoint;
	SpawnInfo marineSpawn;
	SpawnInfo demonSpawn;
	for (int i = 0; i < (int)m_definition->m_spawnInfos.size(); i++)
	{
		if (m_definition->m_spawnInfos[i].m_definition->m_name == "SpawnPoint")
		{
			spawnPoint.push_back(m_definition->m_spawnInfos[i]);
		}
	}
	int randomSpawnPoint = rng.GetRandomIntInRange(0, 1);
	marineSpawn = spawnPoint[randomSpawnPoint];
	Actor* actor = SpawnActor(marineSpawn);
	actor->AddVertsForActor();
	player->Possess(actor);
	player->m_position = actor->m_position;
	actor->m_controller = player;
	player->SetMap(this);
}
void Map::AllActorDestruction()
{
	for (int i = 0; i < (int)m_actors.size(); i++)
	{
		if (m_actors[i])
		{
			if (m_actors[i]->m_isDead && m_actors[i]->m_lifetimeStopwatch.HasDurationElapsed())
			{
				m_actors[i]->m_isDestroyed = true;
			}
		}
	}
}
void Map::AllActorDeletion()
{
	for (int i = 0; i < (int)m_actors.size(); i++)
	{
		if (m_actors[i])
		{
			if (m_actors[i]->m_isDestroyed)
			{
				DestroyActor(m_actors[i]->m_uid);
			}
		}

	}
}
void Map::ChangeLightingSettings()
{
	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F1))
	{
		std::string suntext = Stringf("Sun pitch: %.2f", m_sunPitch);
		DebugAddMessage(suntext, 3.0f, Rgba8::WHITE, Rgba8::WHITE);
		std::string text = Stringf("Ambient intensity: %.2f", m_ambientIntensity);
		DebugAddMessage(text, 3.0f, Rgba8::WHITE, Rgba8::WHITE);
		text = Stringf("Sun intensity: %.2f", m_sunIntensity);
		DebugAddMessage(text, 3.0f, Rgba8::WHITE, Rgba8::WHITE);
	}
	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F2))
	{
		m_ambientIntensity -= 0.1f;
		m_ambientIntensity = Clamp(m_ambientIntensity, 0.0f, 1.0f);
		std::string text = Stringf("Ambient intensity: %.2f", m_ambientIntensity);
		DebugAddMessage(text, 2.0f, Rgba8::WHITE, Rgba8::WHITE);
	}
	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F3))
	{
		m_ambientIntensity += 0.1f;
		m_ambientIntensity = Clamp(m_ambientIntensity, 0.0f, 1.0f);
		std::string text = Stringf("Ambient intensity: %.2f", m_ambientIntensity);
		DebugAddMessage(text, 2.0f, Rgba8::WHITE, Rgba8::WHITE);
	}
	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F4))
	{
		m_sunIntensity -= 0.1f;
		m_sunIntensity = Clamp(m_sunIntensity, 0.0f, 1.0f);
		std::string text = Stringf("Sun intensity: %.2f", m_sunIntensity);
		DebugAddMessage(text, 2.0f, Rgba8::WHITE, Rgba8::WHITE);
	}
	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F5))
	{
		m_sunIntensity += 0.1f;
		m_sunIntensity = Clamp(m_sunIntensity, 0.0f, 1.0f);
		std::string 	text = Stringf("Sun intensity: %.2f", m_sunIntensity);
		DebugAddMessage(text, 2.0f, Rgba8::WHITE, Rgba8::WHITE);
	}
	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F6))
	{
		m_sunPitch -= 10.0f;
		m_sunPitch = Clamp(m_sunPitch, 0.0f, 180.0f);
		std::string text = Stringf("Sun Pitch: %.2f", m_sunPitch);
		DebugAddMessage(text, 2.0f, Rgba8::WHITE, Rgba8::WHITE);
	}
	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F7))
	{
		m_sunPitch += 10.0f;
		m_sunPitch = Clamp(m_sunPitch, 0.0f, 180.0f);
		std::string 	text = Stringf("Sun Pitch: %.2f", m_sunPitch);
		DebugAddMessage(text, 2.0f, Rgba8::WHITE, Rgba8::WHITE);
	}
	g_theRenderer->SetAmbientIntensity(m_ambientIntensity);
	g_theRenderer->SetSunIntensity(m_sunIntensity);
	m_sunDirection.y = RangeMap(m_sunPitch, 0.0f, 180.0f, 0.0f, 1.0f);
	EulerAngles euler = EulerAngles(0.0f, m_sunPitch, 0.0f);
	Mat44 matrix = euler.GetAsMatrix_XFwd_YLeft_ZUp();
	g_theRenderer->SetSunDirection(matrix.GetIBasis3D());
	float sunColor[3];
	sunColor[0] = 0.0f;
	sunColor[1] = 0.0f;
	sunColor[2] = 0.0f;

	g_theRenderer->SetSunColor(sunColor);
}
void Map::AddPointLight(Vec3 Position, float intensity,Rgba8 color, float radius)
{
	g_theRenderer->AddPointLight(Position, intensity, color, radius);
}