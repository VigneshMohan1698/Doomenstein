#include "Player.hpp"
#include "Game/Game.hpp"
#include "Game/Tile.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Game/TileDefinition.hpp"
Tile::Tile(AABB3 bounds, const TileDefinition* definition)
{
	m_bounds = bounds;
	m_definition = definition;
}

bool Tile::IsAir() const
{
	return false;
}

bool Tile::IsSolid() const
{
	if (this == nullptr)
	{
		return false;
	}
	return m_definition->m_isSolid;
}

bool Tile::HasFloor() const
{
	return false;
}
