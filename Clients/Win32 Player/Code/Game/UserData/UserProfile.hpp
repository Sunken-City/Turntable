#pragma once

class UserProfile
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    UserProfile();
    ~UserProfile();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void AddExperience(unsigned int expGained);
    unsigned int CalculateExperienceRequiredForLevel(unsigned int level);
    
private:
    bool CheckForLevelUp();
    unsigned int CalculateLevelFromExperience(unsigned int experience);

    //CONSTANTS/////////////////////////////////////////////////////////////////////
    static constexpr float EXPERIENCE_CURVE_CONSTANT = 0.2f;

public:
    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    unsigned int m_experience = 0;
    unsigned int m_level = 1;
    unsigned int m_numTokens = 0;
};