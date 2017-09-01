#pragma once
#include <string>

class UserProfile
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    UserProfile();
    ~UserProfile();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void AddExperience(unsigned int expGained);
    unsigned int CalculateExperienceRequiredForLevel(unsigned int level);
    void SaveToDisk(const std::string& profileName = "Default");

    //STATIC FUNCTIONS/////////////////////////////////////////////////////////////////////
    static UserProfile* LoadFromDisk(const std::string& profileName = "Default");
    
private:
    bool CheckForLevelUp();
    unsigned int CalculateLevelFromExperience(unsigned int experience);

    //CONSTANTS/////////////////////////////////////////////////////////////////////
    static constexpr float EXPERIENCE_CURVE_CONSTANT = 0.2f;
    static constexpr char* USER_PROFILE_VERSION_STRING = "v0.1";

public:
    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    unsigned int m_experience = 0;
    unsigned int m_level = 1;
    unsigned int m_numTokens = 0;
    unsigned int m_lifetimePlaycounts = 0;
    unsigned int m_lifetimeSecondsListened = 0; //A uint is fine here. Accounts for the next 136 years of listening or so.
};