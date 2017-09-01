#pragma once
#include "Game/UserData/UserProfile.hpp"
#include "Engine/Core/Events/NamedProperties.hpp"

enum ExperienceValues
{
    EXP_FOR_PLAY = 5,
    EXP_FOR_NEW_SONG = 50,
    EXP_FOR_ALBUM = 100,
};

class AchievementManager
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    AchievementManager();
    ~AchievementManager();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    bool LoadDefaultProfile();
    void AddExperience(ExperienceValues expReason, float multiplier = 1.0f);
    void IncrementLifetimeSeconds(float deltaSeconds);
    inline void IncrementLifetimePlaycount() { m_currentProfile->m_lifetimePlaycounts++; };

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static AchievementManager* instance;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    UserProfile* m_currentProfile = nullptr;
};

void OnLevelUp(NamedProperties& params = NamedProperties::NONE);