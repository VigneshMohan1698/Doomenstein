#include "Game/ActorUID.hpp"
#include <Game/GameCommon.hpp>

const ActorUID ActorUID::INVALID(0xffffffff,0xffffffff);
ActorUID::ActorUID()
{
}

ActorUID::ActorUID(int index, int salt)
{
    m_data = index;
    m_data = (m_data << 16) | salt;
}

void ActorUID::Invalidate()
{
    *this = ActorUID::INVALID;
}

bool ActorUID::IsValid() const
{
    return *this != ActorUID::INVALID;
}

int ActorUID::GetIndex() const
{
    return m_data >> 16;
}

bool ActorUID::operator==(const ActorUID& other) const
{
    if(m_data == other.m_data)
    {
        return true;
    }
    return false;
}

bool ActorUID::operator!=(const ActorUID& other) const
{
    UNUSED((void)other);
    return false;
}
