#include "Game/WeaponDefinition.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Game/Actor.hpp"
class Weapon
{
public:
	Weapon( const WeaponDefinition* definition );
	~Weapon();

	void Fire(const Vec3& rayPosition, const Vec3& projectileSpawnPosition, const Vec3& forward, Actor* owner );
	Vec3 GetRandomDirectionInCone( const Vec3& position, const Vec3& forward, float angle ) const;

	const WeaponDefinition* m_definition = nullptr;
	Stopwatch m_refireStopwatch;
};
