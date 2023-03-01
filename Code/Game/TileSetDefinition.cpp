#include "Game/TileSetDefinition.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Game/TileDefinition.hpp"

std::vector<TileSetDefinition*> TileSetDefinition::s_tileSetDefinitions = {};

bool TileSetDefinition::LoadFromXmlElement(const XmlElement& element)
{
	std::string defaultValue = "";
	//Root element
	std::string defaultTile = ParseXmlAttribute(element, "defaultTile", defaultValue);
	std::string defaultName = ParseXmlAttribute(element, "name", defaultValue);
	m_defaultTile = TileDefinition::GetByName(defaultTile);
	m_name = defaultName;

	return false;
}

const TileDefinition* TileSetDefinition::GetTileDefinitionByColor(const Rgba8& color) const
{
	for (int i = 0; i < (int)m_mappings.size(); i++)
	{
		if (m_mappings[i].m_color == color)
		{
			return m_mappings[i].m_tileDefinition;
		}
	}
	return m_defaultTile;
}

void TileSetDefinition::InitializeDefinitions()
{
	tinyxml2::XMLDocument xml_doc;
	tinyxml2::XMLError result = xml_doc.LoadFile("Data/Definitions/TileSetDefinitions.xml");
	if (result != tinyxml2::XML_SUCCESS)
		return;

	//tinyxml2::XMLElement* rootElement = xml_doc.RootElement();
	tinyxml2::XMLElement* tileDefinitionElement = xml_doc.RootElement()->FirstChildElement();
	TileSetDefinition* tileSetdefinition = new TileSetDefinition();
	tileSetdefinition->LoadFromXmlElement(*tileDefinitionElement);

	if (tileDefinitionElement != nullptr)
	{
		for (const tinyxml2::XMLElement* elements = tileDefinitionElement->FirstChildElement(); elements; elements = elements->NextSiblingElement())
		{
			TileMapping* tileMapping = new TileMapping();
			tileMapping->LoadFromXmlElement(*elements);
			tileSetdefinition->m_mappings.push_back(*tileMapping);
		}
	}
	s_tileSetDefinitions.push_back(tileSetdefinition);
	return;
}

const TileSetDefinition* TileSetDefinition::GetByName(const std::string& name)
{
	for (int i = 0; i < (int)s_tileSetDefinitions.size(); i++)
	{
		if (s_tileSetDefinitions[i]->m_name == name)
		{
			return s_tileSetDefinitions[i];
		}
	}
	return nullptr;
}

bool TileMapping::LoadFromXmlElement(const XmlElement& element)
{
	std::string defaultValue = "";
	std::string tileName = ParseXmlAttribute(element, "tile", defaultValue);
	Rgba8 color = ParseXmlAttribute(element, "color", Rgba8::WHITE);
	m_color = color;
	m_tileDefinition = TileDefinition::GetByName(tileName);
	return true;
}
