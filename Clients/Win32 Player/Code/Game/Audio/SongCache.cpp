#include "Game/Audio/SongCache.hpp"
#include "ThirdParty/fmod/fmod.h"
#include "ThirdParty/fmod/fmod.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "../TheGame.hpp"

//-----------------------------------------------------------------------------------
void LoadSongJob(Job* job)
{
    SongResourceInfo* songResource = (SongResourceInfo*)job->data;
    RawSoundHandle song = nullptr;
    unsigned int errorValue = 0;

    while (song == nullptr)
    {
        song = AudioSystem::instance->LoadRawSound(songResource->m_filePath, errorValue);
        if (errorValue == 43)
        {
            Console::instance->PrintLine("ERROR: OUT OF MEMORY, CAN'T LOAD SONG", RGBA::RED);
            return;
        }
        ASSERT_RECOVERABLE(errorValue == 0 || errorValue == 19, "Hit an unexpected error code while loading a file");
    }

    songResource->m_songData = (void*)song;
    songResource->m_timeLastAccessedMS = GetCurrentTimeMilliseconds();
}

//-----------------------------------------------------------------------------------
SongCache::SongCache()
{

}

//-----------------------------------------------------------------------------------
SongCache::~SongCache()
{

}

//-----------------------------------------------------------------------------------
SongID SongCache::RequestSongLoad(const std::wstring& filePath)
{
    SongID songID = SongCache::CalculateSongID(filePath);
    std::map<SongID, SongResourceInfo>::iterator found = m_songCache.find(songID);
    SongResourceInfo* songResourceInfo = nullptr;

    if (found != m_songCache.end())
    {
        songResourceInfo = &found->second;
        songResourceInfo->m_timeLastAccessedMS = GetCurrentTimeMilliseconds();
    }
    else
    {
        m_songCache[songID].m_songID = songID; //Forcibly create a struct of info for the cache
        songResourceInfo = &m_songCache[songID];
        songResourceInfo->m_filePath = filePath;
    }

    JobSystem::instance->CreateAndDispatchJob(GENERIC_SLOW, &LoadSongJob, songResourceInfo);
    return songID;
}

//-----------------------------------------------------------------------------------
RawSoundHandle SongCache::RequestSoundHandle(const SongID songID)
{
    RawSoundHandle song = nullptr;

    std::map<SongID, SongResourceInfo>::iterator found = m_songCache.find(songID);
    if (found != m_songCache.end())
    {
        SongResourceInfo& info = found->second;
        song = info.m_songData;
        info.m_timeLastAccessedMS = GetCurrentTimeMilliseconds();
    }

    return song;
}

//-----------------------------------------------------------------------------------
bool SongCache::IsValid(const SongID songID)
{
    std::map<SongID, SongResourceInfo>::iterator found = m_songCache.find(songID);
    if (found != m_songCache.end())
    {
        SongResourceInfo& info = found->second;
        return info.IsValid();
    }

    return false;
}

//-----------------------------------------------------------------------------------
void SongCache::PrintErrorInConsole(const SongID songID)
{
    std::map<SongID, SongResourceInfo>::iterator found = m_songCache.find(songID);
    if (found != m_songCache.end())
    {
        SongResourceInfo& info = found->second;
        Console::instance->PrintLine(Stringf("AUDIO SYSTEM ERROR: Got error result code %d.\n", info.m_loadErrorCode), RGBA::RED);
    }
}

//-----------------------------------------------------------------------------------
size_t SongCache::CalculateSongID(const std::wstring& filePath)
{
    return std::hash<std::wstring>{}(filePath);
}

//-----------------------------------------------------------------------------------
void SongCache::Flush()
{
    m_songCache.clear();
}

//-----------------------------------------------------------------------------------
SongResourceInfo::~SongResourceInfo()
{
    AudioSystem::instance->ReleaseRawSong(m_songData);
}
