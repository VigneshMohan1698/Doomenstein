#include "Game/TileDefinition.hpp"
#include <Engine/Core/XmlUtils.hpp>
#include "Game/TileMaterialDefinition.hpp"

std::vector<TileDefinition*> TileDefinition::s_tileDefinitions = {};

bool TileDefinition::LoadFromXmlElement(const XmlElement& element)
{
	std::string defaultValue = "";
	std::string tileName = ParseXmlAttribute(element, "name", defaultValue);
	std::string floorMaterial = ParseXmlAttribute(element, "floorMaterial", defaultValue);
	std::string ceilingMaterial = ParseXmlAttribute(element, "ceilingMaterial", defaultValue);
	std::string wallMaterial = ParseXmlAttribute(element, "wallMaterial", defaultValue);

	m_ceilingMaterialDefinition = TileMaterialDefinition::GetByName(ceilingMaterial);
	m_floorMaterialDefinition = TileMaterialDefinition::GetByName(floorMaterial);
	m_wallMaterialDefinition = TileMaterialDefinition::GetByName(wallMaterial);
	bool isSolid = ParseXmlAttribute(element, "isSolid", false);
	m_name = tileName;
	m_isSolid = isSolid;
	return false;
}

void TileDefinition::InitializeDefinitions()
{
	tinyxml2::XMLDocument xml_doc;
	tinyxml2::XMLError result = xml_doc.LoadFile("Data/Definitions/TileDefinitions.xml");
	if (result != tinyxml2::XML_SUCCESS)
		return;

	tinyxml2::XMLElement* rootElement = xml_doc.RootElement();
	if (rootElement != nullptr)
	{
		for (const tinyxml2::XMLElement* elements = rootElement->FirstChildElement(); elements; elements = elements->NextSiblingElement())
		{
			TileDefinition* definition = new TileDefinition();
			definition->LoadFromXmlElement(*elements);
			s_tileDefinitions.push_back(definition);
		}
	}
	return;
}


const TileDefinition* TileDefinition::GetByName(const std::string& name)
{
	for (int i = 0; i < (int)s_tileDefinitions.size(); i++)
	{
		if (s_tileDefinitions[i]->m_name == name)
		{
			return s_tileDefinitions[i];
		}
	}
	return nullptr;
}


