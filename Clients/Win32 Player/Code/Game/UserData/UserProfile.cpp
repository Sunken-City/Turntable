#include "Game/UserData/UserProfile.hpp"
#include <math.h>
#include "Engine/Input/Console.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "AchievementManager.hpp"
#include "Engine/Core/Events/EventSystem.hpp"
#include "ThirdParty/Parsers/XMLParser.hpp"
#include "Engine/Input/XMLUtils.hpp"
#include "Engine/Input/InputOutputUtils.hpp"

//-----------------------------------------------------------------------------------
UserProfile::UserProfile()
{

}

//-----------------------------------------------------------------------------------
UserProfile::~UserProfile()
{
    SaveToDisk();
}

//-----------------------------------------------------------------------------------
void UserProfile::AddExperience(unsigned int expGained)
{
    m_experience += expGained;
    if (CheckForLevelUp())
    {
        unsigned int newLevel = CalculateLevelFromExperience(m_experience);
        m_numTokens += newLevel - m_level;
        m_level = newLevel;
        EventSystem::FireEvent("LevelUp");
    }
}

//-----------------------------------------------------------------------------------
bool UserProfile::CheckForLevelUp()
{
    return (m_experience >= CalculateExperienceRequiredForLevel(m_level + 1));
}

//-----------------------------------------------------------------------------------
unsigned int UserProfile::CalculateExperienceRequiredForLevel(unsigned int level)
{
    float levelOverConstant = (float)level / EXPERIENCE_CURVE_CONSTANT;
    float levelOverConstantSquared = levelOverConstant * levelOverConstant;
    return (unsigned int)ceilf(levelOverConstantSquared);
}

//-----------------------------------------------------------------------------------
unsigned int UserProfile::CalculateLevelFromExperience(unsigned int experience)
{
    return (EXPERIENCE_CURVE_CONSTANT * sqrt(experience));
}

//-----------------------------------------------------------------------------------
void UserProfile::SaveToDisk(const std::string& profileName /*= "Default"*/)
{
    XMLNode root = XMLNode::createXMLTopNode("UserProfile");
    root.addAttribute("Name", profileName.c_str()); //Todo: Add support for multiple profiles.
    root.addAttribute("Version", USER_PROFILE_VERSION_STRING); //Todo: Add support for versioning.
    root.addAttribute("MusicRoot", m_musicRootPath.c_str());
    XMLNode expNode = root.addChild("Stats");
    expNode.addAttribute("Exp", Stringf("%i", m_experience).c_str());
    expNode.addAttribute("Level", Stringf("%i", m_level).c_str());
    expNode.addAttribute("Tokens", Stringf("%i", m_numTokens).c_str());
    expNode.addAttribute("LifetimePlaycounts", Stringf("%i", m_lifetimePlaycounts).c_str());
    expNode.addAttribute("LifetimeSeconds", Stringf("%i", m_lifetimeSecondsListened).c_str());

    std::string saveDirectory = GetAppDataDirectory();
    EnsureDirectoryExists(saveDirectory + "\\Turntable");
    EnsureDirectoryExists(saveDirectory + "\\Turntable\\UserProfiles");
    std::string fullPath = saveDirectory + "\\Turntable\\UserProfiles\\" + profileName + ".xml";
    root.writeToFile(fullPath.c_str());
}

//-----------------------------------------------------------------------------------
UserProfile* UserProfile::LoadFromDisk(const std::string& profileName /*= "Default"*/)
{
    std::string saveDirectory = GetAppDataDirectory();
    std::string fullPath = saveDirectory + "\\Turntable\\UserProfiles\\" + profileName + ".xml";
    if (!FileExists(fullPath))
    {
        return nullptr;
    }

    UserProfile* loadedProfile = new UserProfile();

    XMLNode root = XMLUtils::OpenXMLDocument(fullPath);
    //TODO: check for version attribute and multiple profile names.
    XMLNode userProfile = XMLUtils::GetChildNodeAtPosition(root, "UserProfile");
    const char* musicRoot = userProfile.getAttribute("MusicRoot");
    loadedProfile->m_musicRootPath = musicRoot ? std::string(musicRoot) : "";
    XMLNode expNode = userProfile.getChildNode("Stats");
    loadedProfile->m_experience = std::stoi(expNode.getAttribute("Exp"));
    loadedProfile->m_level = std::stoi(expNode.getAttribute("Level"));
    loadedProfile->m_numTokens = std::stoi(expNode.getAttribute("Tokens"));
    loadedProfile->m_lifetimePlaycounts = std::stoi(expNode.getAttribute("LifetimePlaycounts"));
    loadedProfile->m_lifetimeSecondsListened = std::stoi(expNode.getAttribute("LifetimeSeconds"));

    return loadedProfile;
}

//CONSOLE COMMANDS/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(stats)
{
    UNUSED(args);
    unsigned int secondsListened = AchievementManager::instance->m_currentProfile->m_lifetimeSecondsListened;
    unsigned int level = AchievementManager::instance->m_currentProfile->m_level;

    Console::instance->PrintLine(Stringf("You are level %i [%s]", level, AchievementManager::instance->GetTitleForLevel(level).c_str()), RGBA::CYAN);
    Console::instance->PrintLine(Stringf("You have %i experience.", AchievementManager::instance->m_currentProfile->m_experience), RGBA::PURPLE);
    Console::instance->PrintLine(Stringf("You have %i tokens available to spend.", AchievementManager::instance->m_currentProfile->m_numTokens), RGBA::BADDAD);
    Console::instance->PrintLine(Stringf("You have a total of %i playcounts.", AchievementManager::instance->m_currentProfile->m_lifetimePlaycounts), RGBA::MAGENTA);
    Console::instance->PrintLine(Stringf("You have listened to music for %02i:%02i:%02i.", secondsListened / 3600, (secondsListened / 60) % 60, secondsListened % 60), RGBA::KHAKI);
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(nextlevel)
{
    UNUSED(args);
    unsigned int currentLevel = AchievementManager::instance->m_currentProfile->m_level;
    unsigned int nextLevel = currentLevel + 1;
    unsigned int currentExperience = AchievementManager::instance->m_currentProfile->m_experience;
    unsigned int requiredExperience = AchievementManager::instance->m_currentProfile->CalculateExperienceRequiredForLevel(nextLevel);
    unsigned int differenceExperience = requiredExperience - currentExperience;
    Console::instance->PrintLine(Stringf("You need %i more experience to reach level %i.", differenceExperience, nextLevel), RGBA::PURPLE);
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(addexp)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("addexp <number of experience points to add>", RGBA::RED);
        return;
    }

    AchievementManager::instance->m_currentProfile->AddExperience(args.GetIntArgument(0));
    unsigned int level = AchievementManager::instance->m_currentProfile->m_level;

    Console::instance->PrintLine(Stringf("You are now level %i [%s]", level, AchievementManager::instance->GetTitleForLevel(level).c_str()), RGBA::CYAN);
    Console::instance->PrintLine(Stringf("You now have %i experience.", AchievementManager::instance->m_currentProfile->m_experience), RGBA::PURPLE);
    Console::instance->PrintLine(Stringf("You now have %i tokens available to spend.", AchievementManager::instance->m_currentProfile->m_numTokens), RGBA::BADDAD);
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(printlevels)
{
    for (int i = 0; i < 100; ++i)
    {
        Console::instance->PrintLine(Stringf("Level %i [%s] takes %i exp.", 
            i, 
            AchievementManager::instance->GetTitleForLevel(i).c_str(), 
            AchievementManager::instance->m_currentProfile->CalculateExperienceRequiredForLevel(i)), 
            RGBA::BADDAD);
    }
}