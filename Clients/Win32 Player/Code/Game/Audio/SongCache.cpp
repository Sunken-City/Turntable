#include "Game/Audio/SongCache.hpp"
#include "ThirdParty/fmod/fmod.h"
#include "ThirdParty/fmod/fmod.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Time/Time.hpp"

//-----------------------------------------------------------------------------------
FMOD_RESULT F_CALLBACK DefaultNonblockingCallback(FMOD_SOUND* newSong, FMOD_RESULT result)
{
    ASSERT_OR_DIE(newSong, "Failed to load a song, check the FMOD result and add in error handling for this case <3");

    FMOD::Sound* song = (FMOD::Sound*)newSong; //Converting from the C api to the C++ api
    void* userData = nullptr;
    song->getUserData(&userData);
    SongResourceInfo* songResource = (SongResourceInfo*)userData;
    songResource->m_songData = (void*)song;
    songResource->m_timeLastAccessedMS = GetCurrentTimeMilliseconds();

    return result;
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
    }

    AudioSystem::instance->LoadRawSoundAsync(filePath, &DefaultNonblockingCallback, (void*)songResourceInfo);
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
size_t SongCache::CalculateSongID(const std::wstring& filePath)
{
    return std::hash<std::wstring>{}(filePath);
}
