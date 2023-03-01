#pragma once
#include "Game/ActorUID.hpp"
#include "Game/Map.hpp"
class Actor;

class Controller
{
	friend class Actor;

public:
	Controller();
	virtual ~Controller();

	virtual void Update( float deltaSeconds );
	virtual void WeaponSwitch();
	void Possess( Actor* actor );
	Actor* GetActor() const;
	void SetMap(Map* map);
	virtual void UpdateCamera(float deltaSeconds);
	Map* GetMap();

protected:
	ActorUID m_actorUID = ActorUID::INVALID;
	Map* m_map = nullptr;

};

