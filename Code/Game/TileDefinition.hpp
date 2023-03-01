#pragma once
#include "Game/GameCommon.hpp"
#include <string>
#include <vector>

//------------------------------------------------------------------------------------------------
class TileMaterialDefinition;

//------------------------------------------------------------------------------------------------
class TileDefinition
{
public:
	bool LoadFromXmlElement( const XmlElement& element );

public:
	std::string m_name;
	bool m_isSolid = false;

	const TileMaterialDefinition* m_ceilingMaterialDefinition = nullptr;
	const TileMaterialDefinition* m_floorMaterialDefinition = nullptr;
	const TileMaterialDefinition* m_wallMaterialDefinition = nullptr;

	static void	InitializeDefinitions();
	static const TileDefinition* GetByName( const std::string& name );
	static std::vector<TileDefinition*> s_tileDefinitions;
};