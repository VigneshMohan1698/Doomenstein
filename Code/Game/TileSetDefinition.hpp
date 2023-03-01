#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <string>
#include <vector>

//------------------------------------------------------------------------------------------------
class TileDefinition;

//------------------------------------------------------------------------------------------------
struct TileMapping
{
	bool LoadFromXmlElement( const XmlElement& element );

	Rgba8 m_color;
	const TileDefinition* m_tileDefinition = nullptr;
};

//------------------------------------------------------------------------------------------------
class TileSetDefinition
{
public:
	bool LoadFromXmlElement( const XmlElement& element );

	const TileDefinition* GetTileDefinitionByColor( const Rgba8& color ) const;

public:
	std::string m_name;
	const TileDefinition* m_defaultTile = nullptr;

private:
	std::vector<TileMapping> m_mappings;

public:
	static void InitializeDefinitions();
	static const TileSetDefinition* GetByName( const std::string& name );
	static std::vector<TileSetDefinition*> s_tileSetDefinitions;
};