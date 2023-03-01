#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/AABB3.hpp"

//------------------------------------------------------------------------------------------------
class TileDefinition;

//------------------------------------------------------------------------------------------------
struct Tile
{
public:
	Tile( AABB3 bounds, const TileDefinition* definition = nullptr );
	bool IsAir() const;
	bool IsSolid() const;
	bool HasFloor() const;

public:
	AABB3 m_bounds;
	const TileDefinition* m_definition;
};
