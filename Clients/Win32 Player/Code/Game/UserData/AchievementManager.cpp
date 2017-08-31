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
void AchievementManager::AddExperience(ExperienceValues expReason)
{
    unsigned int experienceGained = (unsigned int)expReason;
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

//EVENTS/////////////////////////////////////////////////////////////////////
void OnLevelUp(NamedProperties& params /*= NamedProperties::NONE*/)
{
    Console::instance->PrintLine(Stringf("CONGRATULATIONS! You are now level %i!", AchievementManager::instance->m_currentProfile->m_level), RGBA::GOLD);
}
