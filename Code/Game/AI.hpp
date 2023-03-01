#pragma once
#include "Engine/Core/Stopwatch.hpp"
#include "Game/Controller.hpp"
struct Vec3;
class AI : public Controller
{
public:
	AI();
	virtual ~AI();

	virtual void Update( float deltaSeconds ) override;
	
	bool	  m_isPlayerVisible;
	Vec3	  m_lastSeenPlayerPosition;
	Stopwatch m_meleeStopwatch;
	Stopwatch m_blueFireStopwatch;
};

