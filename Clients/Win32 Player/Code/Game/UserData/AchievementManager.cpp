#include "Game/UserData/AchievementManager.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Core/Events/EventSystem.hpp"

AchievementManager* AchievementManager::instance = nullptr;

//-----------------------------------------------------------------------------------
AchievementManager::AchievementManager()
{
    EventSystem::RegisterEventCallback("LevelUp", &OnLevelUp);
    if (!LoadDefaultProfile())
    {
        Console::instance->PrintLine("Could not load default profile, creating a new one.", RGBA::RED);
        m_currentProfile = new UserProfile();
    }
}

//-----------------------------------------------------------------------------------
AchievementManager::~AchievementManager()
{
    if (m_currentProfile)
    {
        delete m_currentProfile;
    }
}

//-----------------------------------------------------------------------------------
bool AchievementManager::LoadDefaultProfile()
{
    m_currentProfile = UserProfile::LoadFromDisk();
    return m_currentProfile != nullptr;
}

//-----------------------------------------------------------------------------------
void AchievementManager::AddExperience(ExperienceValues expReason, float multiplier /*= 1.0f*/)
{
    unsigned int experienceGained = (unsigned int)expReason;
    experienceGained = static_cast<unsigned int>((float)experienceGained * multiplier);
    experienceGained = experienceGained == 0 ? 1 : experienceGained;

    switch (expReason)
    {
    case EXP_FOR_PLAY:
        Console::instance->PrintLine(Stringf("Gained %i exp for playing a song.", experienceGained), RGBA::SEA_GREEN);
        break;
    case EXP_FOR_NEW_SONG:
        Console::instance->PrintLine(Stringf("Gained %i exp for playing a brand new song.", experienceGained), RGBA::SEA_GREEN);
        break;
    case EXP_FOR_ALBUM:
        Console::instance->PrintLine(Stringf("Gained %i exp for playing a whole album.", experienceGained), RGBA::SEA_GREEN);
        break;
    default:
        break;
    }

    m_currentProfile->AddExperience(experienceGained);
}

//-----------------------------------------------------------------------------------
void AchievementManager::IncrementLifetimeSeconds(float deltaSeconds)
{
    static float timeListened = 0.0f;
    timeListened += deltaSeconds;
    if (timeListened >= 1.0f)
    {
        //Only add the seconds, leave the fractional portion.
        float secondsListened = floorf(timeListened);
        m_currentProfile->m_lifetimeSecondsListened += (unsigned int)secondsListened;
        timeListened -= secondsListened;
    }
}

//EVENTS/////////////////////////////////////////////////////////////////////
void OnLevelUp(NamedProperties& params /*= NamedProperties::NONE*/)
{
    Console::instance->PrintLine(Stringf("CONGRATULATIONS! You are now level %i!", AchievementManager::instance->m_currentProfile->m_level), RGBA::GOLD);
}
