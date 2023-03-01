#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <string>
#include <vector>

//------------------------------------------------------------------------------------------------
class Shader;
class Texture;

//------------------------------------------------------------------------------------------------
class TileMaterialDefinition
{
public:
	bool LoadFromXmlElement( const XmlElement& element );

public:
	std::string m_name;
	bool m_isVisible = true;
	AABB2 m_uv = AABB2::ZERO_TO_ONE;
	Shader* m_shader = nullptr;
	const Texture* m_texture = nullptr;

	static void InitializeDefinitions();
	static const TileMaterialDefinition* GetByName( const std::string& name );
	static std::vector<TileMaterialDefinition*> s_tileMaterialDefinitions;
};