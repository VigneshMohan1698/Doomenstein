#include "Game/ActorDefinition.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Engine/Renderer/Renderer.hpp"

extern AudioSystem* g_theAudioSystem;
extern Renderer* g_theRenderer;

std::vector<ActorDefinition*> ActorDefinition::s_definitions = {};
bool ActorDefinition::LoadFromXmlElement(const XmlElement& element)
{
	std::string defaultValue = "";
	std::string faction = "";
	std::string inventoryWeapon = "";
	const XmlElement* collisionElement = element.FirstChildElement();
	const XmlElement* physicsElement = nullptr;
	const XmlElement* cameraElement = nullptr;
	const XmlElement* AIelement = nullptr;
	const XmlElement* inventoryElement = nullptr;
	const XmlElement* appearanceElement = nullptr;
	const XmlElement* soundElement = nullptr;
	if (collisionElement)
	{
		physicsElement = collisionElement->NextSiblingElement("Physics");
	}
	if (physicsElement)
	{
		cameraElement = physicsElement->NextSiblingElement("Camera");
	}
	if (collisionElement)
	{
		inventoryElement = collisionElement->NextSiblingElement("Inventory");
	}
	if (collisionElement)
	{
		AIelement = collisionElement->NextSiblingElement("AI");
	}

	appearanceElement = element.FirstChildElement("Appearance");
	soundElement = element.FirstChildElement("Sounds");
	m_name = ParseXmlAttribute(element, "name", defaultValue);
	faction = ParseXmlAttribute(element, "faction", defaultValue);
	m_isVisible = ParseXmlAttribute(element, "visible", true);

	if (faction == "Marine")
	{
		m_faction = Faction::MARINE;
	}
	else if (faction == "Demon")
	{
		m_faction = Faction::DEMON;
	}
	m_health = ParseXmlAttribute(element, "health", 0.0f);
	m_canBePossessed = ParseXmlAttribute(element, "canBePossessed", false);
	m_corpseLifetime = ParseXmlAttribute(element, "corpseLifetime", 0.0f);
	m_dieOnSpawn = ParseXmlAttribute(element, "dieOnSpawn", false);

	if (collisionElement)
	{
		m_physicsRadius = ParseXmlAttribute(*collisionElement, "radius", 0.0f);
		m_physicsHeight = ParseXmlAttribute(*collisionElement, "height", 0.0f);
		m_collidesWithWorld = ParseXmlAttribute(*collisionElement, "collidesWithWorld", false);
		m_collidesWithActors = ParseXmlAttribute(*collisionElement, "collidesWithActors", false);
		m_dieOnCollide = ParseXmlAttribute(*collisionElement, "dieOnCollide", false);
		m_damageOnCollide = ParseXmlAttribute(*collisionElement, "damageOnCollide", FloatRange());
		m_impulseOnCollide = ParseXmlAttribute(*collisionElement, "impulseOnCollide", 0.0f);
	}
	if (physicsElement)
	{
		m_simulated = ParseXmlAttribute(*physicsElement, "simulated", false);
		m_walkSpeed = ParseXmlAttribute(*physicsElement, "walkSpeed", 0.0f);
		m_runSpeed = ParseXmlAttribute(*physicsElement, "runSpeed", 0.0f);
		m_drag = ParseXmlAttribute(*physicsElement, "drag", 0.0f);
		m_turnSpeed = ParseXmlAttribute(*physicsElement, "turnSpeed", 0.0f);
	}
	if (cameraElement)
	{
		m_eyeHeight = ParseXmlAttribute(*cameraElement, "eyeHeight", 0.0f);
		m_cameraFOVDegrees = ParseXmlAttribute(*cameraElement, "cameraFOV", 0.0f);
	}
	if (AIelement)
	{
		m_aiEnabled = ParseXmlAttribute(*AIelement, "aiEnabled", false);
		m_sightRadius = ParseXmlAttribute(*AIelement, "sightRadius", 0.0f);
		m_sightAngle = ParseXmlAttribute(*AIelement, "sightAngle", 0.0f);
		m_meleeDamage = ParseXmlAttribute(*AIelement, "meleeDamage", FloatRange());
		m_meleeDelay = ParseXmlAttribute(*AIelement, "meleeDelay", 0.0f);
		m_meleeRange = ParseXmlAttribute(*AIelement, "meleeRange", 0.0f);
	}
	if (inventoryElement)
	{
 		for (const tinyxml2::XMLElement* elements2 = inventoryElement->FirstChildElement(); elements2; elements2 = elements2->NextSiblingElement())
		{
			inventoryWeapon = ParseXmlAttribute(*elements2, "name", defaultValue);
			const WeaponDefinition* weapon = WeaponDefinition::GetByName(inventoryWeapon);
			m_weaponDefinitions.push_back(weapon);
		}
	}
	if (appearanceElement)
	{
		m_spriteSize = ParseXmlAttribute(*appearanceElement, "size", Vec2());
		m_spritePivot = ParseXmlAttribute(*appearanceElement, "pivot", Vec2());
		std::string billboardType = ParseXmlAttribute(*appearanceElement, "billboardType", defaultValue);
		if (billboardType == "Aligned") { m_billboardType = BillboardType::ALIGNED; };
		if (billboardType == "None") { m_billboardType = BillboardType::NONE; };
		if (billboardType == "Facing") { m_billboardType = BillboardType::FACING; };
		m_renderDepth = ParseXmlAttribute(*appearanceElement, "renderDepth", false);
		m_renderLit = ParseXmlAttribute(*appearanceElement, "renderLit", false);
		m_renderRounded = ParseXmlAttribute(*appearanceElement, "renderRounded", false);
	}

	if (appearanceElement != nullptr)
	{
		for (const tinyxml2::XMLElement* elements = appearanceElement->FirstChildElement(); elements; elements = elements->NextSiblingElement())
		{
			SpriteAnimationGroupDefinition animGroup;
			animGroup.LoadFromXmlElement(*elements, g_theRenderer);
			m_spriteAnimationGroupDefinitions.push_back(animGroup);
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

			if (soundName == "Attack")
			{
				m_attackSound = g_theAudioSystem->CreateOrGetSound(soundPath);
			}
			if (soundName == "Death")
			{
				m_deathSound = g_theAudioSystem->CreateOrGetSound(soundPath);
			}
			if (soundName == "Hurt")
			{
				m_hurtSound = g_theAudioSystem->CreateOrGetSound(soundPath);
			}
		}
	}
	if (m_name == "BloodSplatter")
	{
		m_dieOnCollide = true;
	}
	return false;
}

const SpriteAnimationGroupDefinition* ActorDefinition::GetDefaultAnimationGroup() const
{
	return &m_spriteAnimationGroupDefinitions[0];
}

const SpriteAnimationGroupDefinition* ActorDefinition::GetAnimationGroup(const std::string& name) const
{
	for (int i = 0; i < (int)m_spriteAnimationGroupDefinitions.size(); i++)
	{
		if (m_spriteAnimationGroupDefinitions[i].m_name == name)
		{
			return &m_spriteAnimationGroupDefinitions[i];
		}
	}
	return nullptr;
}

void ActorDefinition::InitializeDefinitions(const char* path)
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
			ActorDefinition* definition = new ActorDefinition();
			definition->LoadFromXmlElement(*elements);
			s_definitions.push_back(definition);
		}
	}
	return;
}

void ActorDefinition::ClearDefinitions()
{
	s_definitions.clear();
}

const ActorDefinition* ActorDefinition::GetByName(const std::string& name)
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
