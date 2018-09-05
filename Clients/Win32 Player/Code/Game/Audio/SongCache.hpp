#pragma once
#include <string>
#include "Engine/Audio/Audio.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "Game/Audio/SongState.hpp"

typedef size_t SongID;

struct SongResourceInfo
{
    ~SongResourceInfo();

    std::wstring m_filePath = L"UNINITIALIZED_PATH";
    SongID m_songID = 0;
    RawSoundHandle m_songData = nullptr;
    double m_timeLastAccessedMS = -1.0f;
    int m_loadErrorCode = 0;
    SongState::State m_status = SongState::NOT_LOADED;

    bool IsValid() { return m_loadErrorCode == 0; };
};

class SongCache
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    SongCache();
    ~SongCache();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    SongID RequestSongLoad(const std::wstring& filePath);
    SongID EnsureSongLoad(const std::wstring& filePath);
    RawSoundHandle RequestSoundHandle(const SongID songID);
    bool IsValid(const SongID songID);
    void PrintErrorInConsole(const SongID songID);
    void Flush();
    void UpdateLastAccessedTime(const SongID songID);
    void TogglePlayingStatus(const SongID songID);
    bool IsLoaded(const SongID songID);
    SongState::State GetState(const SongID songID);
    unsigned int GetSongsInMemoryCount();
    bool RemoveFromCache(const SongID songID);

private:
    SongID CalculateSongID(const std::wstring& filePath);
    SongID FindLeastAccessedSong();
    SongID FindSongToDelete();
    unsigned __int64 GetProcessMemoryBytes();

    //CONSTANTS/////////////////////////////////////////////////////////////////////
    const double SONG_NEVER_ACCESSED = -1.0;
    const SongID INVALID_SONG_ID = 0;
    const unsigned int MAX_MEMORY_THRESHOLD = (unsigned int)8e8; //1 GB limit for win32

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::map<SongID, SongResourceInfo> m_songCache;
    unsigned __int64 m_cacheSizeBytes = 0;
};