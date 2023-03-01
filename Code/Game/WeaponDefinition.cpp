#include "Game/WeaponDefinition.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Game/ActorDefinition.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"

extern Renderer* g_theRenderer;
extern AudioSystem* g_theAudioSystem;
std::vector<WeaponDefinition*> WeaponDefinition::s_definitions = {};
WeaponDefinition::WeaponDefinition()
{
}
bool WeaponDefinition::LoadFromXmlElement(const XmlElement& element)
{
	std::string defaultValue = "";
	std::string projectileActorName = "";
	m_name =							ParseXmlAttribute(element, "name", defaultValue);
	m_refireTime =						ParseXmlAttribute(element, "refireTime", 0.0f);
	m_numRays =							ParseXmlAttribute(element, "numRays", 0);
	m_rayCone =							ParseXmlAttribute(element, "rayCone", 0.0f);
	m_rayRange =						ParseXmlAttribute(element, "rayRange", 0.0f);
	m_rayDamage =						ParseXmlAttribute(element, "rayDamage", FloatRange());
	m_rayImpulse =						ParseXmlAttribute(element, "rayImpulse", 0.0f);
	m_numProjectiles =					ParseXmlAttribute(element, "numProjectiles", 0);
	projectileActorName =			    ParseXmlAttribute(element, "projectileActor", defaultValue);
	m_projectileActorDefinition =		ActorDefinition::GetByName(projectileActorName);
	m_projectileCone =					ParseXmlAttribute(element, "projectileCone", 0.0f);
	m_projectileSpeed =					ParseXmlAttribute(element, "projectileSpeed", 0.0f);

	const XmlElement* HUD = nullptr;
	const XmlElement* soundElement = nullptr;
	HUD = element.FirstChildElement("HUD");
	soundElement = element.FirstChildElement("Sounds");
	std::string shaderName	=				ParseXmlAttribute(*HUD, "shader", defaultValue);
	m_shader =								g_theRenderer->CreateOrGetShader(shaderName.c_str());
	std::string textureName =			    ParseXmlAttribute(*HUD, "baseTexture", defaultValue);
	m_hudBaseTexture =						g_theRenderer->CreateOrGetTextureFromFile(textureName.c_str());
	textureName =							ParseXmlAttribute(*HUD, "reticleTexture", defaultValue);
	m_reticleTexture =						g_theRenderer->CreateOrGetTextureFromFile(textureName.c_str());
	m_reticleSize =							ParseXmlAttribute(*HUD, "reticleSize", Vec2());
	//------------------------FIRE SHADER TEXTURES---------------------------------------
	if (m_name == "Firethrower")
	{
		textureName = ParseXmlAttribute(*HUD, "fireAlphaTexture", defaultValue);
		m_fireAlphaTexture = g_theRenderer->CreateOrGetTextureFromFile(textureName.c_str());
		textureName = ParseXmlAttribute(*HUD, "fireDiffuseTexture", defaultValue);
		m_fireDiffuseTexture = g_theRenderer->CreateOrGetTextureFromFile(textureName.c_str());
		textureName = ParseXmlAttribute(*HUD, "fireNoiseTexture", defaultValue);
		m_fireNoiseTexture = g_theRenderer->CreateOrGetTextureFromFile(textureName.c_str());
		textureName = ParseXmlAttribute(*HUD, "blueFireDiffuseTexture", defaultValue);
		m_blueFireDiffuseTexture = g_theRenderer->CreateOrGetTextureFromFile(textureName.c_str());
		shaderName = ParseXmlAttribute(*HUD, "fireShader", defaultValue);
		m_fireShader = g_theRenderer->CreateOrGetShader(shaderName.c_str());
	}
	
	for (const tinyxml2::XMLElement* elements = HUD->FirstChildElement("Animation"); elements; elements = elements->NextSiblingElement("Animation"))
	{
		std::string textureFilePath = ParseXmlAttribute(*elements, "spriteSheet", defaultValue);
		Texture* texture = g_theRenderer->CreateOrGetTextureFromFile(textureFilePath.c_str());
		IntVec2 gridLayout = ParseXmlAttribute(*elements, "cellCount", IntVec2());
		m_spriteSheet = new SpriteSheet(*texture, gridLayout);
		Shader* shader;
		shaderName = ParseXmlAttribute(*elements, "shader", defaultValue);
		shader = g_theRenderer->CreateOrGetShader(shaderName.c_str());
		std::string animationName;
		animationName = ParseXmlAttribute(*elements, "name", defaultValue);
		float fps = ParseXmlAttribute(*elements, "secondsPerFrame", 0.0f);
		int startFrame = ParseXmlAttribute(*elements, "startFrame", 0);
		int endFrame = ParseXmlAttribute(*elements, "endFrame", 0);
		SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::ONCE;

		if (animationName == "Idle")
		{
			m_idleAnimationDefinition = new SpriteAnimDefinition (*m_spriteSheet, startFrame, endFrame, fps, shader, playbackType);
		}
		else if(animationName == "Attack")
		{
			m_attackAnimationDefinition = new SpriteAnimDefinition(*m_spriteSheet, startFrame, endFrame, fps, shader, playbackType);
		}
			
	}
	if (soundElement != nullptr)
	{
		std::string soundName;
		std::string soundPath;
		for (const tinyxml2::XMLElement* elements = soundElement->FirstChildElement(); elements; elements = elements->NextSiblingElement())
		{
			soundName = ParseXmlAttribute(*elements, "sound", defaultValue);
			soundPath = ParseXmlAttribute(*elements, "name", defaultValue);
			if (soundName == "Fire")
			{
				m_fireSound = g_theAudioSystem->CreateOrGetSound(soundPath);
			}
		}
	}
	return false;
}

void WeaponDefinition::InitializeDefinitions(const char* path)
{
	tinyxml2::XMLDocument xml_doc;
	tinyxml2::XMLError result = xml_doc.LoadFile(path);
	if (result != tinyxml2::XML_SUCCESS)
		return;

	tinyxml2::XMLElement* rootElement = xml_doc.RootElement();
	if (rootElement != nullptr)
	{
		for (const tinyxml2::XMLElement* elements = rootElement->FirstChildElement(); elements; elements = elements->NextSiblingElement())
		{
			WeaponDefinition* definition = new WeaponDefinition();
			definition->LoadFromXmlElement(*elements);
			s_definitions.push_back(definition);
		}
	}
	return;
}

void WeaponDefinition::ClearDefinitions()
{
	s_definitions.clear();
}

const WeaponDefinition* WeaponDefinition::GetByName(const std::string& name)
{
	for (int i = 0; i < (int)s_definitions.size(); i++)
	{
		if (s_definitions[i]->m_name == name)
		{
			return s_definitions[i];
		}
	}
	return nullptr;
}
