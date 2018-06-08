#pragma once
#include <string>
#include "Engine/Audio/Audio.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
//#define PSAPI_VERSION 1
//#include <Psapi.h>
//#pragma comment(lib, "Psapi.lib")

typedef size_t SongID;

enum Status
{
    NOT_LOADED,
    LOADING,
    LOADED,
    PLAYING,
    UNLOADED
};

struct SongResourceInfo
{
    ~SongResourceInfo();

    std::wstring m_filePath = L"UNINITIALIZED_PATH";
    SongID m_songID = 0;
    RawSoundHandle m_songData = nullptr;
    double m_timeLastAccessedMS = -1.0f;
    int m_loadErrorCode = 0;
    Status m_status = NOT_LOADED;

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
    //unsigned int GetCurrentMemoryUsage();

private:
    SongID CalculateSongID(const std::wstring& filePath);
    SongID FindLeastAccessedSong();
    void RemoveFromCache(const SongID songID);

    //CONSTANTS/////////////////////////////////////////////////////////////////////
    const unsigned int MAX_MEMORY_THRESHOLD = (unsigned int)5e8; //500 MB

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::map<SongID, SongResourceInfo> m_songCache;
    unsigned int m_cacheSizeBytes = 0;
};