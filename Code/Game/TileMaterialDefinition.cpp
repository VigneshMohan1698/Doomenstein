#include "Game/TileMaterialDefinition.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/StringUtils.hpp"
extern Renderer* g_theRenderer;

std::vector<TileMaterialDefinition*> TileMaterialDefinition::s_tileMaterialDefinitions = {};
bool TileMaterialDefinition::LoadFromXmlElement(const XmlElement& element)
{
	std::string defaultValue			= "";
	std::string tileName				= ParseXmlAttribute(element, "name", defaultValue);
	std::string shaderName				= ParseXmlAttribute(element, "shader", defaultValue);
	std::string textureFilePath			= ParseXmlAttribute(element, "texture", defaultValue);
	//std::string shaderFilePath			= Stringf("/Data/" + shaderName);
	Vec2 uvs							= ParseXmlAttribute(element, "cell", Vec2());
	bool isVisible					    = ParseXmlAttribute(element, "isVisible", true);
	Shader* shader						= nullptr;
	Texture* texture					= nullptr;
	if (shaderName != "") { shader = g_theRenderer->CreateOrGetShader(shaderName.c_str()); }
	if(textureFilePath!= ""){ texture = g_theRenderer->CreateOrGetTextureFromFile(textureFilePath.c_str()); }
	m_shader							= shader;
	m_texture							= texture;
	m_isVisible							= isVisible;
	m_uv								= AABB2(uvs, uvs+1.0f);
	m_name								= tileName;
	return false;
}

void TileMaterialDefinition::InitializeDefinitions()
{
	tinyxml2::XMLDocument xml_doc;
	tinyxml2::XMLError result = xml_doc.LoadFile("Data/Definitions/TileMaterialDefinitions.xml");
	if (result != tinyxml2::XML_SUCCESS)
		return;

	tinyxml2::XMLElement* rootElement = xml_doc.RootElement();
	if (rootElement != nullptr)
	{
		for (const tinyxml2::XMLElement* elements = rootElement->FirstChildElement(); elements; elements = elements->NextSiblingElement())
		{
			TileMaterialDefinition* tileMaterial =  new TileMaterialDefinition();
			tileMaterial->LoadFromXmlElement(*elements);
			s_tileMaterialDefinitions.push_back(tileMaterial);
		}
	}
	return;
}

const TileMaterialDefinition* TileMaterialDefinition::GetByName(const std::string& name)
{
	for (int i = 0; i < (int)s_tileMaterialDefinitions.size(); i++)
	{
		if (s_tileMaterialDefinitions[i]->m_name == name)
		{
			return s_tileMaterialDefinitions[i];
		}
	}
	return nullptr;
}
