#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/Vec3.hpp"
#include <string>

//------------------------------------------------------------------------------------------------
class TileSetDefinition;
class SpawnInfo;
//------------------------------------------------------------------------------------------------
class MapDefinition
{
public:
	bool LoadFromXmlElement( const XmlElement& element );

public:
	std::string m_name;
	Image m_image;
	const TileSetDefinition* m_tileSetDefinition = nullptr;
	std::vector<SpawnInfo> m_spawnInfos;

	static void InitializeDefinitions();
	static void ClearDefinitions();
	static const MapDefinition* GetByName( const std::string& name );
	static std::vector<MapDefinition*> s_mapDefinitions;


};


