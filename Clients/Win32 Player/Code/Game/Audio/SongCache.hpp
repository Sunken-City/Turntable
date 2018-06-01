#pragma once
#include <string>
#include "Engine/Audio/Audio.hpp"
#include "Engine/Input/InputOutputUtils.hpp"

typedef size_t SongID;

struct SongResourceInfo
{
    ~SongResourceInfo();

    std::wstring m_filePath = L"UNINITIALIZED_PATH";
    SongID m_songID = 0;
    RawSoundHandle m_songData = nullptr;
    double m_timeLastAccessedMS = -1.0f;
    int m_loadErrorCode = 0;

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
    RawSoundHandle RequestSoundHandle(const SongID songID);
    bool IsValid(const SongID songID);
    void PrintErrorInConsole(const SongID songID);
    void Flush();

private:
    SongID CalculateSongID(const std::wstring& filePath);
    SongID FindLeastAccessedSong();
    void RemoveFromCache(const SongID songID);

    //CONSTANTS/////////////////////////////////////////////////////////////////////
    const unsigned int MAX_MEMORY_THRESHOLD = (unsigned int)1e9; //1 GB

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::map<SongID, SongResourceInfo> m_songCache;
    unsigned int m_cacheSizeBytes = 0;
};