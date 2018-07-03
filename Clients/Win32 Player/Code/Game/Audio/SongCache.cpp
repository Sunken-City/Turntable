#include "Game/Audio/SongCache.hpp"
#include "ThirdParty/fmod/fmod.h"
#include "ThirdParty/fmod/fmod.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "../TheGame.hpp"

typedef std::map<SongID, SongResourceInfo>::iterator SongCacheIterator;

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
            songResource->m_status = SongState::CANT_LOAD;
            return;
        }
        ASSERT_RECOVERABLE(errorValue == 0 || errorValue == 19, "Hit an unexpected error code while loading a file");
    }

    songResource->m_songData = (void*)song;
    songResource->m_status = SongState::LOADED;
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
    SongCacheIterator found = m_songCache.find(songID);
    SongResourceInfo* songResourceInfo = nullptr;
    long long fileSize = GetFileSizeBytes(filePath);

    if (found != m_songCache.end())
    {
        songResourceInfo = &found->second;
        if (songResourceInfo->m_songData || songResourceInfo->m_status == SongState::LOADING)
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
    }

    bool canRemove = true;
    while (canRemove && (fileSize + m_cacheSizeBytes) >= MAX_MEMORY_THRESHOLD && GetNumLoadedSongs() > 1)
    {
        //Try to remove already played songs if we can
        canRemove = RemoveFromCache(FindLeastAccessedSong());
    }
    //We want to make the placeholders for the song if we're going to be over the memory threshold. 
    if ((fileSize + m_cacheSizeBytes) >= MAX_MEMORY_THRESHOLD)
    {
        return songID;
    }

    m_cacheSizeBytes += fileSize;
    songResourceInfo->m_status = SongState::LOADING;
    JobSystem::instance->CreateAndDispatchJob(GENERIC_SLOW, &LoadSongJob, songResourceInfo);
    return songID;
}

//-----------------------------------------------------------------------------------
SongID SongCache::EnsureSongLoad(const std::wstring& filePath)
{
    //We need to load the song now, so we delete from the cache if necessary
    SongID songID = SongCache::CalculateSongID(filePath);
    SongCacheIterator found = m_songCache.find(songID);
    SongResourceInfo* songResourceInfo = nullptr;

    if (found != m_songCache.end())
    {
        songResourceInfo = &found->second;
        //If the song is already loaded, return its id
        if (songResourceInfo->m_songData || songResourceInfo->m_status == SongState::LOADING)
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

    long long fileSize = GetFileSizeBytes(filePath);
    bool canRemove = true;
    while (canRemove && (fileSize + m_cacheSizeBytes) >= MAX_MEMORY_THRESHOLD && GetNumLoadedSongs() > 1)
    {
        //Remove the least accessed songs until enough memory is available
        canRemove = RemoveFromCache(FindSongToDelete());
    }

    if ((fileSize + m_cacheSizeBytes) >= MAX_MEMORY_THRESHOLD)
    {
        //File is too large to load into memory
        songResourceInfo->m_status = SongState::CANT_LOAD;
    }
    else
    {
        m_cacheSizeBytes += fileSize;
        songResourceInfo->m_status = SongState::LOADING;
        JobSystem::instance->CreateAndDispatchJob(GENERIC_SLOW, &LoadSongJob, songResourceInfo);
    }

    return songID;
}

//-----------------------------------------------------------------------------------
RawSoundHandle SongCache::RequestSoundHandle(const SongID songID)
{
    RawSoundHandle song = nullptr;

    SongCacheIterator found = m_songCache.find(songID);
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
    SongCacheIterator found = m_songCache.find(songID);
    if (found != m_songCache.end())
    {
        SongResourceInfo& info = found->second;
        if (info.m_status != SongState::CANT_LOAD)
        {
            return info.IsValid();
        }
    }

    return false;
}

//-----------------------------------------------------------------------------------
void SongCache::PrintErrorInConsole(const SongID songID)
{
    SongCacheIterator found = m_songCache.find(songID);
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
    m_cacheSizeBytes = 0;
}

//-----------------------------------------------------------------------------------
SongResourceInfo::~SongResourceInfo()
{
    AudioSystem::instance->ReleaseRawSong(m_songData);
}

//-----------------------------------------------------------------------------------
SongID SongCache::FindLeastAccessedSong()
{
    double lowestAccessTime = SONG_NEVER_ACCESSED;
    SongID leastAccessedSong = INVALID_SONG_ID;

    //Try to find a song that is loaded in and has been played.
    for (SongCacheIterator i = m_songCache.begin(); i != m_songCache.end(); ++i)
    {
        SongResourceInfo& info = i->second;
        bool accessTimeIsLower = ((info.m_timeLastAccessedMS < lowestAccessTime) || lowestAccessTime == SONG_NEVER_ACCESSED) ? true : false;
        bool hasBeenAccessed = info.m_timeLastAccessedMS != SONG_NEVER_ACCESSED ? true : false;
        bool isNotPlaying = info.m_status != SongState::PLAYING ? true : false;

        if (info.m_songData && isNotPlaying && hasBeenAccessed && accessTimeIsLower)
        {
            lowestAccessTime = info.m_timeLastAccessedMS;
            leastAccessedSong = info.m_songID;
        }
    }

    return leastAccessedSong;
}

//-----------------------------------------------------------------------------------
SongID SongCache::FindSongToDelete()
{
    double lowestAccessTime = SONG_NEVER_ACCESSED;
    SongID leastAccessedSong = INVALID_SONG_ID;

    //Find a song that is loaded in and has been played. Otherwise find one that has been loaded
    for (SongCacheIterator i = m_songCache.begin(); i != m_songCache.end(); ++i)
    {
        SongResourceInfo& info = i->second;
        bool accessTimeIsLower = ((info.m_timeLastAccessedMS < lowestAccessTime) || lowestAccessTime == SONG_NEVER_ACCESSED) ? true : false;
        bool hasBeenAccessed = info.m_timeLastAccessedMS != SONG_NEVER_ACCESSED ? true : false;
        bool isNotPlaying = info.m_status != SongState::PLAYING ? true : false;

        if (info.m_songData && isNotPlaying && hasBeenAccessed && accessTimeIsLower)
        {
            lowestAccessTime = info.m_timeLastAccessedMS;
            leastAccessedSong = info.m_songID;
        }
        else if (info.m_songData && isNotPlaying && leastAccessedSong == INVALID_SONG_ID)
        {
            leastAccessedSong = info.m_songID;
        }
    }

    return leastAccessedSong;
}

//-----------------------------------------------------------------------------------
bool SongCache::RemoveFromCache(const SongID songID)
{
    SongCacheIterator found = m_songCache.find(songID);
    if (found != m_songCache.end())
    {
        SongResourceInfo& info = found->second;
        m_cacheSizeBytes -= GetFileSizeBytes(info.m_filePath);
        AudioSystem::instance->ReleaseRawSong(info.m_songData);
        info.m_songData = nullptr;
        info.m_status = SongState::UNLOADED;
        return true;
    }

    return false;
        //ASSERT_OR_DIE(found != m_songCache.end(), "Could not remove song from cache.\n");
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
    SongCacheIterator found = m_songCache.find(songID);
    if (found != m_songCache.end())
    {
        SongResourceInfo& info = found->second;
        info.m_status != SongState::PLAYING ? info.m_status = SongState::PLAYING : info.m_status = SongState::LOADED;
    }
}

//-----------------------------------------------------------------------------------
bool SongCache::IsLoaded(const SongID songID)
{
    SongCacheIterator found = m_songCache.find(songID);
    if (found != m_songCache.end())
    {
        SongResourceInfo& info = found->second;
        if (info.m_status == SongState::PLAYING || info.m_status == SongState::LOADED)
        {
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------------
SongState::State SongCache::GetState(const SongID songID)
{
    SongCacheIterator found = m_songCache.find(songID);
    if (found != m_songCache.end())
    {
        SongResourceInfo& info = found->second;
        return info.m_status;
    }

    return SongState::INVALID_STATE;
}

//-----------------------------------------------------------------------------------
unsigned int SongCache::GetNumLoadedSongs()
{
    //Count if a song is loaded, playing, or loading
    unsigned int numLoadedSongs = 0;
    for (SongCacheIterator i = m_songCache.begin(); i != m_songCache.end(); ++i)
    {
        SongState::State& status = i->second.m_status;
        if (status == SongState::LOADED || status == SongState::PLAYING)
        {
            ++numLoadedSongs;
        }
    }

    return numLoadedSongs;
}