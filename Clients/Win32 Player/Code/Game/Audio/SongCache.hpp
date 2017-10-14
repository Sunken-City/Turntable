#pragma once
#include <string>
#include "Engine/Audio/Audio.hpp"

typedef size_t SongID;

struct SongResourceInfo
{
    SongID m_songID = 0;
    FMOD::Sound* m_songData = nullptr;
    AudioChannelHandle m_audioChannel = nullptr;
};

class SongCache
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    SongCache();
    ~SongCache();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    SongID RequestSongLoad(const std::wstring& filePath);
    AudioChannelHandle RequestChannelHandle(const SongID songID);

private:
    SongID CalculateSongID(const std::wstring& filePath);

    //CONSTANTS/////////////////////////////////////////////////////////////////////
    const unsigned int MAX_MEMORY_THRESHOLD = 1e9;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::map<SongID, SongResourceInfo> m_songCache;
};