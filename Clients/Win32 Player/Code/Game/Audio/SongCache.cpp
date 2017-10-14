#include "Game/Audio/SongCache.hpp"
#include "ThirdParty/fmod/fmod.h"
#include "ThirdParty/fmod/fmod.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//-----------------------------------------------------------------------------------
FMOD_RESULT F_CALLBACK DefaultNonblockingCallback(FMOD_SOUND* newSong, FMOD_RESULT result)
{
    if (newSong)
    {
        FMOD::Sound* song = (FMOD::Sound*)newSong;
        void* userData = nullptr;
        song->getUserData(&userData);
        SongResourceInfo* songResource = (SongResourceInfo*)userData;
        songResource->m_songData = song;
    }
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
    }
    else
    {
        m_songCache[songID].m_songID = songID; //Forcibly create a struct of info for the cache
        songResourceInfo = &m_songCache[songID];
    }

    AudioSystem::instance->LoadAudioChannelAsync(filePath, &DefaultNonblockingCallback, (void*)songResourceInfo);
    return songID;
}

//-----------------------------------------------------------------------------------
AudioChannelHandle SongCache::RequestChannelHandle(const SongID songID)
{
    AudioChannelHandle channel = nullptr;

    std::map<SongID, SongResourceInfo>::iterator found = m_songCache.find(songID);
    if (found != m_songCache.end())
    {
        SongResourceInfo& info = found->second;
        channel = info.m_audioChannel;
    }

    return channel;
}

//-----------------------------------------------------------------------------------
size_t SongCache::CalculateSongID(const std::wstring& filePath)
{
    return std::hash<std::wstring>{}(filePath);
}
