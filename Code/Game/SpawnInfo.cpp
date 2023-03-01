#include "Game/SpawnInfo.hpp"
#include <Engine/Core/XmlUtils.hpp>

SpawnInfo::SpawnInfo()
{
}

SpawnInfo::SpawnInfo(const ActorDefinition* definition, const Vec3& position, const EulerAngles& orientation, const Vec3& velocity)
{
	m_definition = definition;
	m_position = position;
	m_orientation = orientation;
    m_velocity = velocity;
}

SpawnInfo::SpawnInfo(const char* definitionName, const Vec3& position, const EulerAngles& orientation, const Vec3& velocity)
{
	m_definition = ActorDefinition::GetByName(definitionName);
	m_position = position;
	m_orientation = orientation;
	m_velocity = velocity;
}

bool SpawnInfo::LoadFromXmlElement(const XmlElement& element)
{
	std::string defaultValue = "";
	std::string definition = "";
	definition = ParseXmlAttribute(element, "actor", defaultValue);
	m_definition = ActorDefinition::GetByName(definition);
	m_position = ParseXmlAttribute(element, "position", Vec3());
	Vec3 orientation = ParseXmlAttribute(element, "orientation", Vec3());
	m_orientation.m_yawDegrees = orientation.x;
	m_orientation.m_pitchDegrees = orientation.y;
	m_orientation.m_rollDegrees = orientation.z;

    return false;
}
