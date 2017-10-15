#pragma once
#include <string>
#include "Engine/Audio/Audio.hpp"

typedef size_t SongID;

struct SongResourceInfo
{
    SongID m_songID = 0;
    RawSoundHandle m_songData = nullptr;
    double m_timeLastAccessedMS = -1.0f;
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

private:
    SongID CalculateSongID(const std::wstring& filePath);

    //CONSTANTS/////////////////////////////////////////////////////////////////////
    const unsigned int MAX_MEMORY_THRESHOLD = 1e9;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::map<SongID, SongResourceInfo> m_songCache;
};