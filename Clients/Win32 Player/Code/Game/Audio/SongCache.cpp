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
    songResource->m_status = LOADED;
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
    //Load the song into memory completely if it can, otherwise we create the placeholders and load later
    SongID songID = SongCache::CalculateSongID(filePath);
    std::map<SongID, SongResourceInfo>::iterator found = m_songCache.find(songID);
    SongResourceInfo* songResourceInfo = nullptr;

    if (found != m_songCache.end())
    {
        songResourceInfo = &found->second;
        if (songResourceInfo->m_timeLastAccessedMS != SONG_NEVER_ACCESSED && !songResourceInfo->m_songData)
        {
            //If the track has been loaded and played before, and then unloaded, increase the cache size and load again
            m_cacheSizeBytes += GetFileSizeBytes(filePath);
        }
        else if (songResourceInfo->m_songData)
        {
            //The song ID is in the cache and the song is loaded, so we're able to play
            return songID;
        }
    }
    else
    {
        //Song ID wasn't found, so we need to either load the song if we have the space, or create a placeholder and load later
        m_songCache[songID].m_songID = songID; //Forcibly create a struct of info for the cache
        songResourceInfo = &m_songCache[songID];
        songResourceInfo->m_filePath = filePath;

        //We want to make the placeholders for the song if we're going to be over the memory threshold. 
        if (GetFileSizeBytes(filePath) + m_cacheSizeBytes >= MAX_MEMORY_THRESHOLD && m_songCache.size() > 1)
        {
            return songID;
        }
    }

    m_cacheSizeBytes += GetFileSizeBytes(filePath);
    songResourceInfo->m_status = LOADING;
    JobSystem::instance->CreateAndDispatchJob(GENERIC_SLOW, &LoadSongJob, songResourceInfo);
    return songID;
}

//-----------------------------------------------------------------------------------
SongID SongCache::EnsureSongLoad(const std::wstring& filePath)
{
    //We need to load the song now, so we delete from the cache if necessary
    SongID songID = SongCache::CalculateSongID(filePath);
    std::map<SongID, SongResourceInfo>::iterator found = m_songCache.find(songID);
    SongResourceInfo* songResourceInfo = nullptr;

    if (found != m_songCache.end())
    {
        songResourceInfo = &found->second;
        //If the song is loading or is loaded, return its id
        if (songResourceInfo->m_songData || songResourceInfo->m_status == LOADING)
        {
            return songID;
        }
    }
    else
    {
        m_songCache[songID].m_songID = songID; //Forcibly create a struct of info for the cache
        songResourceInfo = &m_songCache[songID];
        songResourceInfo->m_filePath = filePath;
    }

    while (GetFileSizeBytes(filePath) + m_cacheSizeBytes >= MAX_MEMORY_THRESHOLD && m_songCache.size() > 1)
    {
        //Remove the least accessed songs until enough memory is available
        RemoveFromCache(FindLeastAccessedSong());
    }

    if (GetFileSizeBytes(filePath) >= MAX_MEMORY_THRESHOLD)
    {
        //File is too large to load into memory, so we stream it
        songResourceInfo->m_status = CANT_LOAD;
        //SoundID id = AudioSystem::instance->CreateOrGetSound(filePath);
    }
    else
    {
        m_cacheSizeBytes += GetFileSizeBytes(filePath);
        songResourceInfo->m_status = LOADING;
        JobSystem::instance->CreateAndDispatchJob(GENERIC_SLOW, &LoadSongJob, songResourceInfo);
    }

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

//-----------------------------------------------------------------------------------
SongID SongCache::FindLeastAccessedSong()
{
    float lowestAccessTime = SONG_NEVER_ACCESSED;
    SongID leastAccessedSong = 0;

    //Try to find a song that is loaded in and has been played. Otherwise find one that has been loaded
    for (std::map<SongID, SongResourceInfo>::iterator i = m_songCache.begin(); i != m_songCache.end(); ++i)
    {
        SongResourceInfo& info = i->second;
        //Has this song been loaded yet? Has it been loaded and not played? Is the last load time less than what we have?
        if (((info.m_timeLastAccessedMS < lowestAccessTime) || lowestAccessTime == SONG_NEVER_ACCESSED) && info.m_timeLastAccessedMS != SONG_NEVER_ACCESSED && info.m_songData && info.m_status != PLAYING)
        {
            lowestAccessTime = info.m_timeLastAccessedMS;
            leastAccessedSong = info.m_songID;
        }
        else if (info.m_songData && info.m_status != PLAYING && leastAccessedSong == SONG_NEVER_ACCESSED)
        {
            leastAccessedSong = info.m_songID;
        }
    }

    return leastAccessedSong;
}

//-----------------------------------------------------------------------------------
void SongCache::RemoveFromCache(const SongID songID)
{
    if (songID == 0)
    {
        return;
    }

    std::map<SongID, SongResourceInfo>::iterator found = m_songCache.find(songID);
    if (found != m_songCache.end())
    {
        SongResourceInfo& info = found->second;
        m_cacheSizeBytes -= GetFileSizeBytes(info.m_filePath);
        AudioSystem::instance->ReleaseRawSong(info.m_songData);
        info.m_songData = nullptr;
        info.m_status = UNLOADED;
    }
    else
    {
        ASSERT_RECOVERABLE(found == m_songCache.end(), "Could not remove song from cache.\n");
    }
}

//-----------------------------------------------------------------------------------
void SongCache::UpdateLastAccessedTime(const SongID songID)
{
    SongResourceInfo& info = m_songCache.at(songID);
    info.m_timeLastAccessedMS = GetCurrentTimeMilliseconds();
}

//-----------------------------------------------------------------------------------
void SongCache::TogglePlayingStatus(const SongID songID)
{
    std::map<SongID, SongResourceInfo>::iterator found = m_songCache.find(songID);
    if (found != m_songCache.end())
    {
        SongResourceInfo& info = found->second;
        if (info.m_status != PLAYING)
        {
            info.m_status = PLAYING;
        }
        else
        {
            info.m_status = LOADED;
        }
    }
}

//-----------------------------------------------------------------------------------
bool SongCache::IsLoaded(const SongID songID)
{
    std::map<SongID, SongResourceInfo>::iterator found = m_songCache.find(songID);
    if (found != m_songCache.end())
    {
        SongResourceInfo& info = found->second;
        if (info.m_status == PLAYING || info.m_status == LOADED)
        {
            return true;
        }
    }
    return false;
}

