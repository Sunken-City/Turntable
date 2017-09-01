#pragma once
#include "Game/UserData/UserProfile.hpp"
#include "Engine/Core/Events/NamedProperties.hpp"

enum ExperienceValues
{
    EXP_FOR_PLAY = 10,
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
    void AddExperience(ExperienceValues expReason);

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static AchievementManager* instance;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    UserProfile* m_currentProfile = nullptr;
};

void OnLevelUp(NamedProperties& params = NamedProperties::NONE);