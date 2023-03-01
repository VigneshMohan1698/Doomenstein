#include "MapDefinition.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Image.hpp"
#include "Game/TileSetDefinition.hpp"
#include "Game/SpawnInfo.hpp"

extern Renderer* g_theRenderer;
std::vector<MapDefinition*> MapDefinition::s_mapDefinitions = {};

bool MapDefinition::LoadFromXmlElement(const XmlElement& element)
{
	std::string defaultValue = "";
	std::string tileName  =		ParseXmlAttribute(element, "name", defaultValue);
	std::string imagePath =		ParseXmlAttribute(element, "image", defaultValue);
	std::string tileSet   =		ParseXmlAttribute(element, "tileSet", defaultValue);
	const XmlElement* spawnInfoElement = element.FirstChildElement();

	for (const tinyxml2::XMLElement* elements = spawnInfoElement->FirstChildElement(); elements; elements = elements->NextSiblingElement())
	{
		SpawnInfo info = SpawnInfo();
		info.LoadFromXmlElement(*elements);
		m_spawnInfos.push_back(info);
	}

	m_tileSetDefinition = TileSetDefinition::GetByName(tileSet.c_str());
	Image image = Image(imagePath.c_str());
	m_name = tileName;
	m_image = image;
	return false;
}

void MapDefinition::InitializeDefinitions()
{
	tinyxml2::XMLDocument xml_doc;
	tinyxml2::XMLError result = xml_doc.LoadFile("Data/Definitions/MapDefinitions.xml");
	if (result != tinyxml2::XML_SUCCESS)
		return;

	tinyxml2::XMLElement* rootElement = xml_doc.RootElement();
	if (rootElement != nullptr)
	{
		for (const tinyxml2::XMLElement* elements = rootElement->FirstChildElement(); elements; elements = elements->NextSiblingElement())
		{
			MapDefinition* definition = new MapDefinition();
			definition->LoadFromXmlElement(*elements);
			s_mapDefinitions.push_back(definition);
		}
	}
	return;
}

void MapDefinition::ClearDefinitions()
{
}

const MapDefinition* MapDefinition::GetByName(const std::string& name)
{
	for (int i = 0; i < (int)s_mapDefinitions.size(); i++)
	{
		if (s_mapDefinitions[i]->m_name == name)
		{
			return s_mapDefinitions[i];
		}
	}
	return nullptr;
}
