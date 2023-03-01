#include "Game/Controller.hpp"
Controller::Controller()
{
}



Controller::~Controller()
{
}

void Controller::Update(float deltaSeconds)
{
    UNUSED((void)deltaSeconds);
}

void Controller::WeaponSwitch()
{
}

void Controller::Possess(Actor* actor)
{
    m_actorUID = actor->m_uid;
}

Actor* Controller::GetActor() const
{
    int actorIndex = m_actorUID.GetIndex();
    return m_map->m_actors[actorIndex];
}

void Controller::SetMap(Map* map)
{
    m_map = map;
}
void Controller::UpdateCamera(float deltaSeconds)
{
    UNUSED((void)deltaSeconds);
}

Map* Controller::GetMap()
{
    return m_map;
}